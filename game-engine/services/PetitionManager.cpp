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

<<<<<<< Updated upstream
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
=======
    for(auto& petition : petitionsToRemove) {
        underConstructionPetitions.erase(
            std::remove(underConstructionPetitions.begin(), underConstructionPetitions.end(), petition),
            underConstructionPetitions.end()
        );

        delete petition;
    }

    return completedEffects; // Could be empty if no petitions completed this tick
}

void PetitionManager::acceptPetition()
{
    if (currentPetition != nullptr)
    {
        underConstructionPetitions.push_back(currentPetition);
        currentPetition = generatePetition();
    }
}

void PetitionManager::rejectPetition()
{
    if (currentPetition != nullptr)
    {
        delete currentPetition;
        currentPetition = generatePetition();
    }
}

Petition* PetitionManager::getCurrentPetition() const
{
    return currentPetition;
}

Petition* PetitionManager::generatePetition() // This needs to be implemented
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 3);
    int num = dis(gen);

    switch(num){
        case 1: //we get a petition for energy
        {
            Building* b = new PowerPlant();
            Petition* p = new Petition(1, b);
            return p;
        }
        case 2: //we get a petition for water
        {
            
        }
        case 3: //we get a petition for money
        {
            
        }
    }


}
>>>>>>> Stashed changes
