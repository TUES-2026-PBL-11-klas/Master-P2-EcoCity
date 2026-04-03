#include "GameService.hpp"

GameService::GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city)
    : resourceManager(resourceManager), petitionManager(petitionManager), city(city) {}

bool GameService::tick()
{
    const std::vector<ResourceEffect> completedEffects = petitionManager->tick();
    resourceManager->applyEffect(completedEffects);
    return checkGameOver();
}

void GameService::readPlayerInput()
{
    // Input handling is not implemented yet.
}

bool GameService::checkGameOver()
{
    return resourceManager->tick();
}
