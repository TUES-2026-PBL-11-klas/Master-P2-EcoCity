#include <vector>
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"
#include "domain/Petition.hpp"

class PetitionManager {
    private:
        int petitionCount;
        std::vector<Petition*> underConstructionPetitions;
    public:
        PetitionManager()
        {
            petitionCount = 0;
            underConstructionPetitions.clear();
        }
        std::vector<ResourceEffect> tick()
        {
            std::vector<Petition*> petitionsToRemove;
            std::vector<ResourceEffect> completedEffects;
            for (auto& petition : underConstructionPetitions) {
                petition->decreaseTicksToComplete();
                if(petition->getTicksToComplete() <= 0){
                    completedEffects.insert(completedEffects.end(), petition->getEffects().begin(), petition->getEffects().end());
                    petitionsToRemove.push_back(petition);
                }
            }

            for(auto& petition : petitionsToRemove) {
                underConstructionPetitions.erase(
                    std::remove(underConstructionPetitions.begin(), underConstructionPetitions.end(), petition),
                    underConstructionPetitions.end()
                );
            }
            
            return completedEffects;
        }
        void generatePetition();
        void removePetition(int id);
        void addPetition(Petition* petition);
};
