#ifndef MONGO_GAME_REPOSITORY_H
#define MONGO_GAME_REPOSITORY_H

#include <string>

#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/instance.hpp>
#include <mongocxx/v_noabi/mongocxx/uri.hpp>

#include "../domain/City.hpp"
#include "../services/PetitionManager.hpp"
#include "../services/ResourceManager.hpp"

class MongoGameRepository {
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
        );
};

#endif
