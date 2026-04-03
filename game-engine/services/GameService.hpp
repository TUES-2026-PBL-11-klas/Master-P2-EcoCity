#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include "IGameService.hpp"

#include "ResourceManager.hpp"
#include "PetitionManager.hpp"
#include "../domain/City.hpp"

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
