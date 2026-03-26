#include <vector>
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"
#include "domain/Petition.hpp"//might need to change these

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
            for (auto& petition : underConstructionPetitions) {
                petition->changeTicksToComplete(petition->getTicksToComplete() - 1/*this is gonna change*/);
                if(petition->getTicksToComplete() <= 0){
                    petition->getEffects();//this needs logic to be finished
                    underConstructionPetitions.erase(std::remove(underConstructionPetitions.begin(), underConstructionPetitions.end(), petition), underConstructionPetitions.end());
                }
            }
        }
        std::vector<ResourceEffect> getEffects(int id);
        void generatePetition();
        void removePetition(int id);
        void addPetition(Petition* petition);
};