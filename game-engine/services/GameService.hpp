#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include "IGameService.hpp"
#include "ResourceManager.hpp"
#include "PetitionManager.hpp"
#include "../domain/City.hpp"
#include "../network/ISocketServer.hpp"
#include "../persistence/IGameRepository.hpp"

#define MAX_CO2 100'000'000LL

class GameService : public IGameService {
    private:
        ResourceManager* resourceManager;
        PetitionManager* petitionManager;
        City* city;
        ISocketServer* socketServer;
        IGameRepository* gameRepository;
        std::string gameId;

        bool checkGameOver();
        game_api::v1::GameState buildGameState() const;

    public:
        GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city, ISocketServer* socketServer,
                    IGameRepository* gameRepository, const std::string& gameId);
        bool tick() override;
        void readPlayerInput() override;
};

#endif
