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

    // Resources: one entry per type
    struct ResourceData {
        ResourceType type = RESOURCE_UNSPECIFIED;
        long long amount = 0;
        long long changesPerTick = 0;
    };
    std::vector<ResourceData> resources;

    // Current petition shown to the player
    struct PetitionData {
        int id = 0;
        BuildingType buildingType = BUILDING_UNSPECIFIED;
        long long cost = 0;
        int ticksRemaining = 0;   // ticks_needed_for_construction as saved
    };

    bool hasCurrentPetition = false;
    PetitionData currentPetition;

    // Petitions currently under construction
    std::vector<PetitionData> underConstructionPetitions;

    // How many of each building type have been completed
    std::unordered_map<BuildingType, int> buildingCounts;
};

class MongoGameRepository : public IGameRepository {
    private:
        mongocxx::instance instance;
        mongocxx::client client;
        std::string databaseName;

    public:
        MongoGameRepository(const std::string& connectionString, const std::string& databaseName);

        void saveGame(
            const std::string& gameId,
            const ResourceManager& resourceManager,
            const PetitionManager& petitionManager,
            const City& city
        ) override;

        // Returns a SavedGame. Check .found to know if any save existed.
        SavedGame loadGame(const std::string& gameId);
};

#endif
