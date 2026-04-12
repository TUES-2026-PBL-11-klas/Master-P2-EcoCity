#include "GameService.hpp"
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

GameService::GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city)
: resourceManager(resourceManager), petitionManager(petitionManager), city(city) {}

bool GameService::tick()
{
    readPlayerInput();

    const std::vector<CompletedConstruction> completedConstructions = petitionManager->tick();
    for (const CompletedConstruction& construction : completedConstructions) {
        resourceManager->applyEffect(construction.effects);
        city->addBuilding(construction.type);
    }

    resourceManager->tick();

    printResourceSnapshot(*resourceManager);

    return checkGameOver();
}

bool GameService::checkGameOver()
{
    for(const Resource& resource : *resourceManager)
    {
        if (resource.getCurrentValue() <= 0 && resource.getType() != CO2) {
            return true;    // Game over if any resource except CO2 is depleted
        }
    }

    return resourceManager->getResourceValue(CO2) >= MAX_CO2; // Game over if CO2 reaches limit
}

void GameService::readPlayerInput()
{
    return; // Placeholder for future implementation of player input handling
}
