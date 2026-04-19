#ifndef MONGO_GAME_REPOSITORY_H
#define MONGO_GAME_REPOSITORY_H

#include <string>

#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/instance.hpp>
#include <mongocxx/v_noabi/mongocxx/uri.hpp>

#include "../domain/City.hpp"
#include "IGameRepository.hpp"
#include "../services/PetitionManager.hpp"
#include "../services/ResourceManager.hpp"

struct SavedGame {
    bool found = false;

    struct ResourceData {
        ResourceType type = RESOURCE_UNSPECIFIED;
        long long amount = 0;
        long long changesPerTick = 0;
    };
    std::vector<ResourceData> resources;

    struct PetitionData {
        int id = 0;
        BuildingType buildingType = BUILDING_UNSPECIFIED;
        long long cost = 0;
        int ticksRemaining = 0;
    };

    bool hasCurrentPetition = false;
    PetitionData currentPetition;
    std::vector<PetitionData> underConstructionPetitions;
    std::unordered_map<BuildingType, int> buildingCounts;
};

class MongoGameRepository : public IGameRepository {
    private:
        mongocxx::client client;
        std::string databaseName;

    public:
        MongoGameRepository(const std::string& connectionString, const std::string& databaseName);

        void saveGame(
            const std::string& gameId,
            const ResourceManager& resourceManager,
            const PetitionManager& petitionManager,
            const City& city, const std::string& traceId=""
        ) override;

        SavedGame loadGame(const std::string& gameId);
};

#endif
