#include <iostream>
#include <thread>
#include <chrono>

#include "services/GameService.hpp"
#include "services/ResourceManager.hpp"
#include "services/PetitionManager.hpp"
#include "domain/City.hpp"
#include "domain/buildings/BuildingFactory.hpp"
#include "persistence/MongoGameRepository.hpp"
#include "network/SocketServer.hpp"

// Restoring petitions under construction is the trickiest part because Building
// counts down ticksToComplete internally.  We create a fresh building with
// createBuilding() (which sets the *full* tick cost) and then call buildTick()
// repeatedly until the remaining counter matches what was saved.  That keeps
// all the Building subclass logic intact without touching private members.

// Applies a SavedGame snapshot back onto the live objects so the game continues exactly where it was saved
static void restoreGame(
    const SavedGame& save,
    ResourceManager& resourceManager,
    PetitionManager& petitionManager,
    City& city)
{
    if (!save.found) return;

    // Resources
    for (const auto& rd : save.resources)
    {
        if (rd.type == RESOURCE_UNSPECIFIED) continue;

        // Bring current value to the saved amount
        long long current = resourceManager.getResourceValue(rd.type);
        resourceManager.changeResourceValue(rd.type, rd.amount - current);

        resourceManager.setDeltaForResourceType(rd.type, rd.changesPerTick);
    }

    // Building counts (City)
    for (const auto& [bt, count] : save.buildingCounts)
    {
        for (int i = 0; i < count; ++i) {
            city.addBuilding(bt);
        }
    }

    // Petitions under construction
    for (const auto& pd : save.underConstructionPetitions)
    {
        Building* building = createBuilding(pd.buildingType);
        if (building == nullptr) continue;

        building->setTicksToComplete(pd.ticksRemaining);

        // Wrap in a Petition and inject it directly into PetitionManager.
        Petition* petition = new Petition(pd.id, building);
        petitionManager.restoreUnderConstruction(petition);
    }

    // Current petition
    if (save.hasCurrentPetition) {
        const auto& pd = save.currentPetition;
        Building* building = createBuilding(pd.buildingType);

        if (building != nullptr) {
            building->setTicksToComplete(pd.ticksRemaining);
            building->setBuildCost(pd.cost);
        }

        Petition* petition = new Petition(pd.id, building);
        petitionManager.restoreCurrentPetition(petition);
    }

    std::cout << "[Restore] Game state applied successfully.\n";
}

int main() {
    const std::string mongoConnectionString = "mongodb://localhost:27017/";
    const std::string databaseName = "Eco_city_game";
    const std::string gameId = "local_game";
    const int uiPort = 54321;

    ResourceManager resourceManager;
    PetitionManager petitionManager;
    City city;

    SocketServer socketServer(uiPort);

    MongoGameRepository gameRepository(mongoConnectionString, databaseName);

    // Try to load an existing save
    SavedGame save = gameRepository.loadGame(gameId);
    if (save.found)
    {
        std::cout << "[Main] Resuming saved game...\n";
        restoreGame(save, resourceManager, petitionManager, city);
    }
    else
    {
        std::cout << "[Main] No save found, starting fresh game.\n";
        gameRepository.saveGame(gameId, resourceManager, petitionManager, city);
        std::cout << "[Main] Saved initial game state with game_id=" << gameId << "\n";
    }

    // gameRepository.saveGame(gameId, resourceManager, petitionManager, city);
    // std::cout << "Saved initial game state to MongoDB with game_id=" << gameId << std::endl;

    GameService gameService(&resourceManager, &petitionManager, &city, &socketServer, &gameRepository, gameId);

    bool endGame = false;
    while (!endGame)
    {
        endGame = gameService.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
