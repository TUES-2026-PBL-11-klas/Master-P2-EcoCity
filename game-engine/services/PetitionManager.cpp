#include <vector>
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"
#include "domain/Petition.hpp"
#include "services/PetitionManager.hpp"

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
        void tick()
        {
            std::vector<Petition*> petitionsToRemove;
            for (auto& petition : underConstructionPetitions) {
                petition->decreaseTicksToComplete();
                if(petition->getTicksToComplete() <= 0){
                    petition->getEffects();//this needs logic to be finished
                    // Currently, we just remove the petition from the list when it's completed.
                    // In a real implementation, we would also return the effects to the caller
                    petitionsToRemove.push_back(petition);
                }
            }

            for(auto& petition : petitionsToRemove) {
                underConstructionPetitions.erase(
                    std::remove(underConstructionPetitions.begin(), underConstructionPetitions.end(), petition),
                    underConstructionPetitions.end()
                );
            }
        }
        std::vector<ResourceEffect> getEffects(int id);
        void generatePetition();
        void removePetition(int id);
        void addPetition(Petition* petition);
};
