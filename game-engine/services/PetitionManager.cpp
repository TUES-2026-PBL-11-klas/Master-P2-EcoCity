#include "PetitionManager.hpp"

#include "../domain/buildings/PowerPlant.hpp" // There might be a better way to do this than including all the building types here

PetitionManager::PetitionManager()
{
    currentPetition = generatePetition();
}

// Returns effects of the completed petitions
std::vector<ResourceEffect> PetitionManager::tick()
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

Petition* PetitionManager::generatePetition() // This needs to be implemented. Placeholder, should return a new Petition based on game logic
{
    static int nextId = 1;
    Building* building = new PowerPlant();
    return new Petition(nextId++, building);
}

PetitionManager::~PetitionManager() {
    delete currentPetition;
    for (Petition* petition : underConstructionPetitions)
    {
        delete petition;
    }
}
