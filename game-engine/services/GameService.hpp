#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include "IGameService.hpp"

#include "ResourceManager.hpp"
#include "PetitionManager.hpp"
#include "../domain/City.hpp"

#define MAX_CO2 100'000'000LL

class GameService : public IGameService {
    private:
        ResourceManager* resourceManager;
        PetitionManager* petitionManager;
        City* city;

        bool checkGameOver();

    public:
        GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city);
        bool tick() override;
        void readPlayerInput() override;
};

#endif
