#ifndef PETITION_MANAGER_H
#define PETITION_MANAGER_H

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "../domain/Petition.hpp"
// #include "../domain/buildings/BuildingFactory.hpp"
#include "../domain/buildings/PowerPlant.hpp" // Temporary, should be replaced with a more flexible solution for generating petitions with different building types

struct CompletedConstruction {
    BuildingType type;
    std::vector<ResourceEffect> effects;
};

class PetitionManager {
    private:
        Petition* currentPetition;
        std::vector<Petition*> underConstructionPetitions;
        std::mt19937 randomEngine;
        int nextPetitionId;

    public:
        PetitionManager();
        std::vector<CompletedConstruction> tick();
        void acceptPetition();
        void rejectPetition();
        Petition* generatePetition();
        Petition* getCurrentPetition() const;
        ~PetitionManager();

        PetitionManager(const PetitionManager&) = delete;
        PetitionManager& operator=(const PetitionManager&) = delete;
};

#endif
