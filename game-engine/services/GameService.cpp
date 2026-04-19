#include "GameService.hpp"
#include "../observability/Logger.hpp"
#include "../observability/Tracer.hpp"
#include "../exceptions/InsufficientResourcesException.hpp"
#include "../exceptions/PersistenceException.hpp"

#include <filesystem>
#include <iostream>

namespace {
void printResourceSnapshot(const ResourceManager& resourceManager)
{
    std::cout << " | Money (100k GBP): " << resourceManager.getResourceValue(MONEY)
              << " | Energy (MWh): " << resourceManager.getResourceValue(ENERGY)
              << " | Water (kL): " << resourceManager.getResourceValue(WATER)
              << " | CO2 (5 t): " << resourceManager.getResourceValue(CO2)
              << " | Population: " << resourceManager.getResourceValue(POPULATION)
              << '\n';
}
}

GameService::GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city, ISocketServer* socketServer,
    IGameRepository* gameRepository, const std::string& gameId)
: resourceManager(resourceManager), petitionManager(petitionManager), city(city), socketServer(socketServer),
gameRepository(gameRepository), gameId(gameId)
{
    auto now = std::chrono::system_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();

    std::filesystem::path metricsDir = std::filesystem::path(PROJECT_SOURCE_DIR) / "metrics";
    std::filesystem::create_directories(metricsDir);

    std::string filename = (metricsDir / ("resources_" + std::to_string(ms) + ".csv")).string();
    metricsFile_.open(filename, std::ios::app);
    if (!metricsFile_.is_open()) {
        throw std::runtime_error("Failed to open resource metrics file: " + filename);
    }

    filename = (metricsDir / ("system_" + std::to_string(ms) + ".csv")).string();
    systemMetricsFile_.open(filename, std::ios::app);
    if (!systemMetricsFile_.is_open()) {
        throw std::runtime_error("Failed to open system metrics file: " + filename);
    }

    if (metricsFile_.tellp() == 0) {
        metricsFile_ << "tick,money,energy,water,co2,population\n";
    }

    if (systemMetricsFile_.tellp() == 0) {
        systemMetricsFile_ << "tick,cpu_percent,mem_kb,thread_count\n";
    }
}

bool GameService::tick()
{
    TRACE("GameService", "tick");

    currentTraceId_ = generateTraceId();
    LOG_INFO("GameService", "tick_start", "trace_id=" + currentTraceId_);
    std::cout << "Tick " << tickCount_ + 1 << " - Trace ID: " << currentTraceId_ << std::endl;

    try {
        readPlayerInput();
    } catch (const InsufficientResourcesException& e) {
        LOG_WARN("GameService", "petition_insufficient_funds", e.what());
    }

    const std::vector<CompletedConstruction> completedConstructions = petitionManager->tick(currentTraceId_);
    std::for_each(completedConstructions.begin(),
              completedConstructions.end(),
              [&](const CompletedConstruction& construction)
    {
        resourceManager->applyEffect(construction.effects);
        city->addBuilding(construction.type);
    });

    resourceManager->tick(currentTraceId_);
    handlePopulationScaling();

    socketServer->sendGameState(buildGameState());

    printResourceSnapshot(*resourceManager);

    LOG_DEBUG("GameService", "tick_complete", "trace_id=" + currentTraceId_);

    metricsFile_ << ++tickCount_
    << "," << resourceManager->getResourceValue(MONEY)
    << "," << resourceManager->getResourceValue(ENERGY)
    << "," << resourceManager->getResourceValue(WATER)
    << "," << resourceManager->getResourceValue(CO2)
    << "," << resourceManager->getResourceValue(POPULATION)
    << "\n";
    metricsFile_.flush();

    if(tickCount_ % 10 == 0)
    {
        ProcStat curr = readProcStat();
        SystemMetrics metrics = readSystemMetrics(lastProcStat_, curr);
        lastProcStat_ = curr;

        systemMetricsFile_
            << tickCount_          << ","
            << metrics.cpuPercent  << ","
            << metrics.memUsedKB   << ","
            << metrics.threadCount << "\n";
        systemMetricsFile_.flush();

        if (metrics.memUsedKB > 500000)
            LOG_WARN("GameService", "high_memory", "mem_kb=" + std::to_string(metrics.memUsedKB));
        if (metrics.cpuPercent > 80.0)
            LOG_WARN("GameService", "high_cpu", "cpu_pct=" + std::to_string(metrics.cpuPercent));
    }

    return checkGameOver();
}

bool GameService::checkGameOver()
{
    bool hasInvalidResource = std::any_of(
    resourceManager->begin(),
    resourceManager->end(),
    [&](const Resource& resource)
    {
        return resource.getCurrentValue() <= 0 &&
               resource.getType() != CO2;
    });

    if (hasInvalidResource)
    {
        LOG_WARN("GameService", "game_over", "reason=resource_depleted");
        return true;
    }

    if(resourceManager->getResourceValue(CO2) >= MAX_CO2)
    {
        LOG_WARN("GameService", "game_over", "reason=co2_limit_exceeded");
        return true;
    }

    return false;
}

void GameService::readPlayerInput()
{
    auto action = socketServer->pollAction();
    if (!action.has_value()) return;
    const game_api::v1::UIAction& uiAction = action.value();

    if (uiAction.has_petition_response()) {
        const auto& response = uiAction.petition_response();
        if (response.responded()) {
            if (response.accepted()) {
                // Guard: verify the player can afford the building before accepting.
                // Throws InsufficientResourcesException if funds are too low, so the
                // game loop rejects the action cleanly rather than silently overdrafting.
                const Petition* petition = petitionManager->getCurrentPetition();
                if (petition != nullptr && petition->getBuilding() != nullptr) {
                    LLint cost      = petition->getBuilding()->getBuildCost();
                    LLint available = resourceManager->getResourceValue(ResourceType::MONEY);
                    if (!resourceManager->canAfford(cost)) {
                        throw InsufficientResourcesException(cost, available);
                    }
                }
                petitionManager->acceptPetition();
                LOG_INFO("GameService", "petition_accepted");
            } else {
                petitionManager->rejectPetition();
                LOG_INFO("GameService", "petition_rejected");
            }
        }
    }

    if (uiAction.save_game()) {
        try {
            gameRepository->saveGame(gameId, *resourceManager, *petitionManager, *city, currentTraceId_);
            LOG_INFO("GameService", "game_saved");
        } catch (const PersistenceException& e) {
            LOG_ERROR("GameService", "save_failed", e.what());
        }
    }
}

game_api::v1::GameState GameService::buildGameState() const
{
    game_api::v1::GameState state;

   std::for_each(city->getBuildings().begin(),
              city->getBuildings().end(),
              [&](const auto& entry)
    {
        const auto& [type, count] = entry;
        (*state.mutable_building_counts())[static_cast<int>(type)] = count;
    });

    const Petition* petition = petitionManager->getCurrentPetition();
    if (petition != nullptr && petition->getBuilding() != nullptr) {
        game_api::v1::Petition* protoPetition = state.mutable_current_petition();
        protoPetition->set_id(petition->getId());

        game_api::v1::Building* protoBuilding = protoPetition->mutable_building();
        const Building* building = petition->getBuilding();
        protoBuilding->set_type(
            static_cast<game_api::v1::BuildingType>(building->getType()));
        protoBuilding->set_build_cost(static_cast<int32_t>(building->getBuildCost()));
        protoBuilding->set_ticks_to_complete(building->getTicksToComplete());

        for (const ResourceEffect& effect : building->getEffects()) {
            game_api::v1::ResourceEffect* protoEffect = protoBuilding->add_effects();
            protoEffect->set_resource_type(
                static_cast<game_api::v1::ResourceType>(effect.type));
            protoEffect->set_delta_value(static_cast<int32_t>(effect.deltaValue));
        }
    }

    for (const Resource& resource : *resourceManager) {
        (*state.mutable_resources())[resource.getType()] =
            static_cast<int32_t>(resource.getCurrentValue());
    }

    return state;
}

void GameService::handlePopulationScaling()
{
    long long currentPop = resourceManager->getResourceValue(POPULATION);

    if (currentPop < nextPopulationGoal)
        return;

    nextPopulationGoal = static_cast<long long>(nextPopulationGoal * SCALING_FACTOR);

    auto applyDeltaRule = [&](ResourceType type, long long currentDelta) -> long long
    {
        if (type == WATER || type == ENERGY)
        {
            return (currentDelta < 0)
                ? static_cast<long long>(currentDelta * demandIncrease)
                : static_cast<long long>(currentDelta * (2 - demandIncrease));
        }

        if (type == CO2)
        {
            return (currentDelta > 0)
                ? static_cast<long long>(currentDelta * demandIncrease)
                : currentDelta - static_cast<long long>(currentDelta * (2 - demandIncrease));
        }

        if (type == MONEY)
        {
            return static_cast<long long>(currentDelta * demandIncrease);
        }

        return currentDelta;
    };

    for (auto& resource : *resourceManager)
    {
        ResourceType type = resource.getType();
        long long currentDelta = resource.getDeltaValue();

        long long newDelta = applyDeltaRule(type, currentDelta);

        resourceManager->setDeltaForResourceType(type, newDelta);
    }

    LOG_DEBUG("GameService",
              "population_milestone_reached",
              "new_goal=" + std::to_string(nextPopulationGoal));
}
