#include <vector>
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"
#include "domain/Petition.hpp"

class PetitionManager {
    private:
        int petitionCount;
        Petition* currentPetition;
        std::vector<Petition*> underConstructionPetitions;

    public:
        PetitionManager()
        {
            petitionCount = 0;
            currentPetition = generatePetition();
        }

        // Returns effects of the completed petitions
        std::vector<ResourceEffect> tick()
        {
            std::vector<Petition*> petitionsToRemove;
            std::vector<ResourceEffect> completedEffects;
            for (auto& petition : underConstructionPetitions) {
                std::vector<ResourceEffect> effects = petition->buildTick();

                if(!effects.empty()) {
                    completedEffects.insert(completedEffects.end(), effects.begin(), effects.end());
                    petitionsToRemove.push_back(petition);
                }
            }

            for(auto& petition : petitionsToRemove) {
                underConstructionPetitions.erase(
                    std::remove(underConstructionPetitions.begin(), underConstructionPetitions.end(), petition),
                    underConstructionPetitions.end()
                );

                delete petition;
            }

            return completedEffects; // Could be empty if no petitions completed this tick
        }

        void acceptPetition()
        {
            if (currentPetition != nullptr)
            {
                underConstructionPetitions.push_back(currentPetition);
                currentPetition = generatePetition();
            }
        }

        void rejectPetition()
        {
            if (currentPetition != nullptr)
            {
                delete currentPetition;
                currentPetition = generatePetition();
            }
        }

        Petition* getCurrentPetition() const
        {
            return currentPetition;
        }

        Petition* generatePetition(); // This needs to be implemented
};
