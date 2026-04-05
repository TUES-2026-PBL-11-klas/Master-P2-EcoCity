#ifndef PETITION_MANAGER_H
#define PETITION_MANAGER_H

#include <vector>
#include <random>
#include <algorithm>
#include "../domain/buildings/PowerPlant.hpp"
#include "../domain/Petition.hpp"

class PetitionManager {
    private:
        Petition* currentPetition;
        std::vector<Petition*> underConstructionPetitions;

    public:
        PetitionManager();
        std::vector<ResourceEffect> tick();
        void acceptPetition();
        void rejectPetition();
        Petition* generatePetition();
        Petition* getCurrentPetition() const;
};

#endif