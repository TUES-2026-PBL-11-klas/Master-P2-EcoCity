#include "GameService.hpp"

GameService::GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city)
{
    this->resourceManager = resourceManager;
    this->petitionManager = petitionManager;
    this->city = city;
}
bool GameService::tick()
{
    std::vector<ResourceEffect> petitionEffects = petitionManager->tick();
    resourceManager->applyEffect(petitionEffects);
    resourceManager->tick();
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

    return resourceManager->getResourceValue(CO2) >= 10000; // Game over if CO2 reaches 10000
}

void GameService::readPlayerInput()
{
    return; // Placeholder for future implementation of player input handling
}
