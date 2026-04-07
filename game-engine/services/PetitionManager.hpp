#ifndef PETITION_MANAGER_H
#define PETITION_MANAGER_H

#include <random>
#include <vector>

#include "../domain/Petition.hpp"

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
        ~PetitionManager();
        std::vector<CompletedConstruction> tick();
        Petition* acceptPetition();
        void rejectPetition();
        Petition* generatePetition();
        Petition* getCurrentPetition() const;
};

#endif
