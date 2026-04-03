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
    bool gameOver = resourceManager->tick();//Putting a comment here so we do not
    //forget this might not need to be checking game over.
    //Also maybe we should modify the entire chain of functions
    //because i am not sure if Resource.changeCurrentValue works as intendded
    //especially for CO2.
    return gameOver;
}

// bool GameService::checkGameOver()
// {
//     if(this->resourceManager->getResourceValue(WATER) <= 0 ||
//         this->resourceManager->getResourceValue(ENERGY) <= 0 ||
//         this->resourceManager->getResourceValue(MONEY) <= 0 ||
//         this->resourceManager->getResourceValue(CO2) >= 10000)
//     {
//         return true;
//     }
//     else return false;
// }

void GameService::readPlayerInput()
{
    return; // Placeholder for future implementation of player input handling
}
