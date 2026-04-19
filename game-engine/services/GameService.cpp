#include "GameService.hpp"
#include "../observability/Logger.hpp"
#include "../observability/Tracer.hpp"
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

    std::filesystem::create_directories("metrics");

    std::string filename = "metrics/resources_" + std::to_string(ms) + ".csv";
    metricsFile_.open(filename, std::ios::app);
    if (!metricsFile_.is_open()) {
        throw std::runtime_error("Failed to open resource metrics file: " + filename);
    }

    filename = "metrics/system_" + std::to_string(ms) + ".csv";
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

    readPlayerInput();

    const std::vector<CompletedConstruction> completedConstructions = petitionManager->tick();
    for (const CompletedConstruction& construction : completedConstructions) {
        resourceManager->applyEffect(construction.effects);
        city->addBuilding(construction.type);
    }

    resourceManager->tick();
    handlePopulationScaling();

    socketServer->sendGameState(buildGameState());

    printResourceSnapshot(*resourceManager);

    LOG_DEBUG("GameService", "tick_complete");

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
    for(const Resource& resource : *resourceManager)
    {
        if (resource.getCurrentValue() <= 0 && resource.getType() != CO2) {
            LOG_WARN("GameService", "game_over", "reason=resource_depleted");
            return true;
        }
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
                petitionManager->acceptPetition();
                LOG_INFO("GameService", "petition_accepted");
            } else {
                petitionManager->rejectPetition();
                LOG_INFO("GameService", "petition_rejected");
            }
        }
    }

    if (uiAction.save_game()) {
        gameRepository->saveGame(gameId, *resourceManager, *petitionManager, *city);
        LOG_INFO("GameService", "game_saved");
    }
}

game_api::v1::GameState GameService::buildGameState() const
{
    game_api::v1::GameState state;

    for (const auto& [type, count] : city->getBuildings()) {
        (*state.mutable_building_counts())[static_cast<int>(type)] = count;
    }

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

void GameService::handlePopulationScaling() {
    long long int currentPop = resourceManager->getResourceValue(POPULATION);

    if (currentPop >= nextPopulationGoal) {
        nextPopulationGoal = static_cast<long long int>(nextPopulationGoal * SCALING_FACTOR);

        for (auto& resource : *resourceManager) {
            ResourceType type = resource.getType();
            long long int newDelta;
            long long int currentDelta = resource.getDeltaValue();
            if (type == WATER || type == ENERGY) {
                if(currentDelta < 0){
                    newDelta = static_cast<long long int>(currentDelta * demandIncrease);
                }
                else{
                    newDelta = static_cast<long long int>(currentDelta * (2 - demandIncrease));
                }
                resourceManager->setDeltaForResourceType(type, newDelta);
            }
            else if (type == CO2) {
                if(currentDelta > 0)
                {
                    newDelta = static_cast<long long int>(currentDelta * demandIncrease);
                    resourceManager->setDeltaForResourceType(type, newDelta);
                }
                else {
                    newDelta = currentDelta - static_cast<long long int>(currentDelta * (2 - demandIncrease));
                    resourceManager->setDeltaForResourceType(type, newDelta);
                }
            }
            else if(type == MONEY)
            {
                newDelta = static_cast<long long int>(currentDelta * demandIncrease);
                resourceManager->setDeltaForResourceType(type, newDelta);
            }
        }
        LOG_DEBUG("GameService", "population_milestone_reached", "new_goal=" + std::to_string(nextPopulationGoal));
    }
}
