#ifndef PETITION_MANAGER_H
#define PETITION_MANAGER_H

#include <vector>
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"
#include "domain/Petition.hpp"

class PetitionManager {
    private:
        int petitionCount;
        std::vector<Petition*> underConstructionPetitions;
    public:
        PetitionManager();
        std::vector<ResourceEffect> tick();
        void generatePetition();
        void removePetition(int id);
        void addPetition(Petition* petition);
};

#endif
