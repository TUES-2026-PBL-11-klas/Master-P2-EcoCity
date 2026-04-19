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
        const std::vector<Petition*>& getUnderConstructionPetitions() const;

        // Takes ownership of the petition and places it in the under-construction queue
        // Used only during game-state restore from MongoDB
        void restoreUnderConstruction(Petition* petition);

        // Replaces the current petition with the one loaded from MongoDB
        // Deletes any petition that was auto-generated at construction time
        // Takes ownership of the supplied petition
        void restoreCurrentPetition(Petition* petition);

        ~PetitionManager();

        PetitionManager(const PetitionManager&) = delete;
        PetitionManager& operator=(const PetitionManager&) = delete;
};

#endif
