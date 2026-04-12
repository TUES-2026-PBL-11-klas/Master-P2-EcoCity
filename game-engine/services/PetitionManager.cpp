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

const std::vector<Petition*>& PetitionManager::getUnderConstructionPetitions() const
{
    return underConstructionPetitions;
}

Petition* PetitionManager::generatePetition()
{
    static const std::array<BuildingType, 12> buildingPool = {
        POWER_PLANT,
        WATER_TREATMENT_PLANT,
        SOLAR_PANEL_FARM,
        SOLAR_PANEL_ROOFTOPS,
        PUBLIC_TRANSPORT_UPGRADE,
        WIND_TURBINE_FARM,
        HYDROELECTRIC_PLANT,
        URBAN_GREENING,
        WATER_SAVING_INFRASTRUCTURE,
        INDUSTRIAL_ZONE,
        AIRPORT_EXPANSION,
        ROAD_IMPROVEMENT
    };

    std::uniform_int_distribution<std::size_t> distribution(0, buildingPool.size() - 1);
    const BuildingType buildingType = buildingPool[distribution(randomEngine)];

    return new Petition(nextPetitionId++, createBuilding(buildingType));
}

PetitionManager::~PetitionManager() {
    delete currentPetition;
    for (Petition* petition : underConstructionPetitions)
    {
        delete petition;
    }
}
