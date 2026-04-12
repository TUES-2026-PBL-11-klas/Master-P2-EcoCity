#include <iostream>
#include <thread>
#include <chrono>

#include "services/ResourceManager.hpp"
#include "services/PetitionManager.hpp"
#include "services/GameService.hpp"
#include "domain/City.hpp"
#include "persistence/MongoGameRepository.hpp"

int main() {
    const std::string mongoConnectionString = "mongodb://localhost:27017/";
    const std::string databaseName = "Eco_city_game";
    const std::string gameId = "local_game";

    ResourceManager resourceManager;
    PetitionManager petitionManager;
    City city;

    GameService gameService(&resourceManager, &petitionManager, &city);
    MongoGameRepository gameRepository(mongoConnectionString, databaseName);

    gameRepository.saveGame(gameId, resourceManager, petitionManager, city);
    std::cout << "Saved initial game state to MongoDB with game_id=" << gameId << std::endl;

    bool endGame = false;
    while (!endGame)
    {
        endGame = gameService.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
