#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include "IGameService.hpp"
#include "ResourceManager.hpp"
#include "PetitionManager.hpp"
#include "../domain/City.hpp"

#include "../network/SocketServer.hpp"
#include "../persistence/MongoGameRepository.hpp"
#include "../exceptions/InsufficientResourcesException.hpp"

#include "../network/ISocketServer.hpp"
#include "../persistence/IGameRepository.hpp"
#include "../observability/SystemMetrics.hpp"


#include <fstream>

#define MAX_CO2 100'000'000LL
const double SCALING_FACTOR = 1.2;
const double demandIncrease = 1.10;

class GameService : public IGameService {
    private:
        ResourceManager* resourceManager;
        PetitionManager* petitionManager;
        City* city;
        ISocketServer* socketServer;
        IGameRepository* gameRepository;
        std::string gameId;

        std::ofstream metricsFile_;
        std::ofstream systemMetricsFile_;
        ProcStat lastProcStat_ = readProcStat();
        long long tickCount_ = 0;
        long long int nextPopulationGoal = 1200000;

        bool checkGameOver();
        game_api::v1::GameState buildGameState() const;

    public:
        GameService(ResourceManager* resourceManager, PetitionManager* petitionManager, City* city, ISocketServer* socketServer,
                    IGameRepository* gameRepository, const std::string& gameId);
        bool tick() override;
        void readPlayerInput() override;
        void handlePopulationScaling();
};

#endif
