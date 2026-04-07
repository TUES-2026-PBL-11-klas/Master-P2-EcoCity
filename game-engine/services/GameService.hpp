#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include <cstdint>

#include "ResourceManager.hpp"
#include "PetitionManager.hpp"
#include "../domain/City.hpp"

class GameService {
    private:
        ResourceManager* resourceManager;
        PetitionManager* petitionManager;
        City* city;
        int currentYear;
        int maxYears;
    public:
        GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city);
        bool tick();
        void readPlayerInput();
        bool checkGameOver();
};

#endif
