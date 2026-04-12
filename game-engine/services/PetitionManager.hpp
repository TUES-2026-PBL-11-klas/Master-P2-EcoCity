#ifndef PETITION_MANAGER_H
#define PETITION_MANAGER_H

#include <algorithm>
#include <random>
#include <utility>
#include <vector>
#include <array>

#include "../domain/Petition.hpp"
#include "../domain/buildings/BuildingFactory.hpp"

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
