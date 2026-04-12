#include "PetitionManager.hpp"

PetitionManager::PetitionManager()
: currentPetition(nullptr), randomEngine(std::random_device{}()), nextPetitionId(1)
{
    currentPetition = generatePetition();
}

// Returns effects of the completed petitions
std::vector<CompletedConstruction> PetitionManager::tick()
{
    std::vector<Petition*> petitionsToRemove;
    std::vector<CompletedConstruction> completedConstructions;
    for (auto& petition : underConstructionPetitions) {
        std::vector<ResourceEffect> effects = petition->buildTick();

        if(!effects.empty()) {
            completedConstructions.push_back({ petition->getBuilding()->getType(), std::move(effects) });
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

    return completedConstructions; // Could be empty if no petitions completed this tick
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
    Building* building = new PowerPlant();
    return new Petition(nextPetitionId++, building);
}

PetitionManager::~PetitionManager() {
    delete currentPetition;
    for (Petition* petition : underConstructionPetitions)
    {
        delete petition;
    }
}
