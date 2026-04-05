#ifndef PETITION_MANAGER_H
#define PETITION_MANAGER_H

#include <vector>
<<<<<<< Updated upstream
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"
#include "domain/Petition.hpp"
=======
#include "../domain/Petition.hpp"
#include "../domain/Resource.hpp"
#include "../domain/buildings/PowerPlant.hpp"
#include <random>
#include <algorithm>
>>>>>>> Stashed changes

class PetitionManager {
    private:
        int petitionCount;
        std::vector<Petition*> underConstructionPetitions;
    public:
        PetitionManager();
        std::vector<ResourceEffect> tick();
<<<<<<< Updated upstream
        void generatePetition();
        void removePetition(int id);
        void addPetition(Petition* petition);
=======
        void acceptPetition();
        void rejectPetition();
        Petition* generatePetition(); // This needs to be implemented
        Petition* getCurrentPetition() const;
>>>>>>> Stashed changes
};

#endif
