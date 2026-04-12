#include<iostream>
#include <thread>
#include <chrono>

#include "services/ResourceManager.hpp"
#include "services/PetitionManager.hpp"
#include "services/GameService.hpp"
#include "domain/City.hpp"

int main() {
    ResourceManager resourceManager;
    PetitionManager petitionManager;
    City city;

    GameService gameService(&resourceManager, &petitionManager, &city);

    bool endGame = false;
    while (!endGame)
    {
        endGame = gameService.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Sleep for 250 milliseconds to simulate time passing in the game
    }

    return 0;
}
