#ifndef PETITION_MANAGER_H
#define PETITION_MANAGER_H

#include <algorithm>
#include <vector>
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"
#include "domain/Petition.hpp"

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
