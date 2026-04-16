#include "GameService.hpp"
#include "../Logger.hpp"
#include "../Tracer.hpp"
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

GameService::GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city, SocketServer* socketServer,
    MongoGameRepository* gameRepository, const std::string& gameId, std::ofstream metricsFile_)
: resourceManager(resourceManager), petitionManager(petitionManager), city(city), socketServer(socketServer),
gameRepository(gameRepository), gameId(gameId), metricsFile_("metrics.csv", std::ios::app)
{
    // Write CSV header if file is new/empty
    if (metricsFile_.tellp() == 0) {
        metricsFile_ << "tick,money,energy,water,co2,population\n";
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

    // Send updated game state to UI after every tick
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

    return checkGameOver();
}

bool GameService::checkGameOver()
{
    for(const Resource& resource : *resourceManager)
    {
        if (resource.getCurrentValue() <= 0 && resource.getType() != CO2) {
            LOG_WARN("GameService", "game_over", "reason=resource_depleted");
            return true;    // Game over if any resource except CO2 is depleted
        }
    }

    if(resourceManager->getResourceValue(CO2) >= MAX_CO2) // Game over if CO2 reaches limit
    {
        LOG_WARN("GameService", "game_over", "reason=co2_limit_exceeded");
        return true;
    }

    return false;   // Game continues otherwise
}

void GameService::readPlayerInput()
{
    auto action = socketServer->pollAction();   // returns optional if empty returns immediately, else we extract the UIAction
    if (!action.has_value()) return;
    const game_api::v1::UIAction& uiAction = action.value();

    if (uiAction.has_petition_response()) {
        const auto& response = uiAction.petition_response();
        if (response.responded()) {
            if (response.accepted()) {
                petitionManager->acceptPetition();
                // std::cout << "[Input] Petition accepted.\n";
                LOG_INFO("GameService", "petition_accepted");
            } else {
                petitionManager->rejectPetition();
                // std::cout << "[Input] Petition rejected.\n";
                LOG_INFO("GameService", "petition_rejected");
            }
        }
    }

    if (uiAction.save_game()) {
        gameRepository->saveGame(gameId, *resourceManager, *petitionManager, *city);
        // std::cout << "[Input] Game saved to MongoDB.\n";
        LOG_INFO("GameService", "game_saved");
    }
}

game_api::v1::GameState GameService::buildGameState() const
{
    game_api::v1::GameState state;

    // Building counts from City
    for (const auto& [type, count] : city->getBuildings()) {
        (*state.mutable_building_counts())[static_cast<int>(type)] = count;
    }

    // Current petition
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
        // std::cout << "[Balance] Population reached milestone! New goal: " << nextPopulationGoal << "\n";
        LOG_DEBUG("GameService", "population_milestone_reached", "new_goal=" + std::to_string(nextPopulationGoal));
    }
}
