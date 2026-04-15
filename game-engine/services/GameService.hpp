#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include "IGameService.hpp"
#include "ResourceManager.hpp"
#include "PetitionManager.hpp"
#include "../domain/City.hpp"
#include "../network/SocketServer.hpp"
#include "../persistence/MongoGameRepository.hpp"

#define MAX_CO2 100'000'000LL
const double SCALING_FACTOR = 1.2;

class GameService : public IGameService {
    private:
        ResourceManager* resourceManager;
        PetitionManager* petitionManager;
        City* city;
        SocketServer* socketServer;
        MongoGameRepository* gameRepository;
        std::string gameId;
        long long int nextPopulationGoal = 1200000;//population starts at 1 mil

        bool checkGameOver();
        game_api::v1::GameState buildGameState() const;

    public:
        GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city, SocketServer* socketServer,
                    MongoGameRepository* gameRepository, const std::string& gameId);
        bool tick() override;
        void readPlayerInput() override;
        void handlePopulationScaling();
};

#endif