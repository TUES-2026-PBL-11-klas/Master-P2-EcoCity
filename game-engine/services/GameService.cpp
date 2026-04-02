#include "ResourceManager.hpp"
#include "PetitionManager.hpp"
#include "domain/City.hpp"

class GameService {
    private:
        ResourceManager* resourceManager;
        PetitionManager* petitionManager;
        City* city;
    public:
        GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city)
        {
            this->resourceManager = resourceManager;
            this->petitionManager = petitionManager;
            this->city = city;
        }
        bool tick()
        {
            std::vector<ResourceEffect> petitionEffects = petitionManager->tick();
            resourceManager->applyEffect(petitionEffects);
            bool gameOver = resourceManager->tick();//Putting a comment here so we do not
            //forget this might not need to be checking game over. 
            //Also maybe we should modify the entire chain of functions
            //because i am not sure if Resource.changeCurrentValue works as intendded
            //especially for CO2.
            return checkGameOver();
        }
        void readPlayerInput();
        bool checkGameOver()
        {
            if(this->resourceManager->getResourceValue(WATER) <= 0 ||
               this->resourceManager->getResourceValue(ENERGY) <= 0 ||
               this->resourceManager->getResourceValue(MONEY) <= 0 ||
               this->resourceManager->getResourceValue(CO2) >= 10000)
            {
                return true;
            }
            else return false;
        }
};