#include "GameService.hpp"

#include <iostream>

namespace {
void printResourceSnapshot(const ResourceManager& resourceManager, int year)
{
    std::cout << "Year " << year
              << " | Money: " << resourceManager.getResourceValue(MONEY)
              << " | Energy: " << resourceManager.getResourceValue(ENERGY)
              << " | Water: " << resourceManager.getResourceValue(WATER)
              << " | CO2: " << resourceManager.getResourceValue(CO2)
              << " | Population: " << resourceManager.getResourceValue(POPULATION)
              << '\n';
}
}

GameService::GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city)
    : resourceManager(resourceManager), petitionManager(petitionManager), city(city), currentYear(0), maxYears(20) {}

bool GameService::tick()
{
    ++currentYear;

    readPlayerInput();

    const std::vector<CompletedConstruction> completedConstructions = petitionManager->tick();
    for (const CompletedConstruction& construction : completedConstructions) {
        resourceManager->applyEffect(construction.effects);
        city->addBuilding(construction.type);
    }

    const bool gameOver = resourceManager->tick() || checkGameOver();
    printResourceSnapshot(*resourceManager, currentYear);

    return gameOver || currentYear >= maxYears;
}

void GameService::readPlayerInput()
{
    Petition* currentPetition = petitionManager->getCurrentPetition();
    if (currentPetition == nullptr) {
        return;
    }

    const std::int64_t buildCost = currentPetition->getBuilding()->getBuildCost();
    if (!resourceManager->canAfford(buildCost)) {
        petitionManager->rejectPetition();
        return;
    }

    resourceManager->changeResourceValue(MONEY, -buildCost);
    petitionManager->acceptPetition();
}

bool GameService::checkGameOver()
{
    return resourceManager->getResourceValue(MONEY) <= 0
        || resourceManager->getResourceValue(WATER) <= 0
        || resourceManager->getResourceValue(ENERGY) <= 0
        || resourceManager->getResourceValue(POPULATION) <= 0;
}
