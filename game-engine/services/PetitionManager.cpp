#include "PetitionManager.hpp"
#include "../observability/Logger.hpp"
#include "../observability/Tracer.hpp"

PetitionManager::PetitionManager()
: currentPetition(nullptr), randomEngine(std::random_device{}()), nextPetitionId(1)
{
    currentPetition = generatePetition();
}

// Returns effects of the completed petitions
std::vector<CompletedConstruction> PetitionManager::tick(const std::string& traceId)
{
    TRACE("PetitionManager", "tick");

    LOG_INFO("PetitionManager", "tick_start", "trace_id=" + traceId);

    std::vector<Petition*> petitionsToRemove;
    std::vector<CompletedConstruction> completedConstructions;

    std::for_each(underConstructionPetitions.begin(),
                  underConstructionPetitions.end(),
                  [&](Petition* petition)
    {
        auto effects = petition->buildTick();

        if (!effects.empty())
        {
            completedConstructions.push_back(
                { petition->getBuilding()->getType(), std::move(effects) });

            petitionsToRemove.push_back(petition);

            LOG_INFO("PetitionManager", "petition_completed",
                     "id=" + std::to_string(petition->getId()));
        }
    });

    auto it = std::remove_if(
    underConstructionPetitions.begin(),
    underConstructionPetitions.end(),
    [&](Petition* p)
    {
        return std::find(petitionsToRemove.begin(),
                         petitionsToRemove.end(),
                         p) != petitionsToRemove.end();
    });

    underConstructionPetitions.erase(it, underConstructionPetitions.end());

    for (Petition* p : petitionsToRemove)
    {
        delete p;
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

   auto pickRandomBuilding = [&]()
    {
        std::uniform_int_distribution<std::size_t> dist(0, buildingPool.size() - 1);
        return buildingPool[dist(randomEngine)];
    };

    const BuildingType buildingType = pickRandomBuilding();

    LOG_DEBUG("PetitionManager", "petition_generated", "id=" + std::to_string(nextPetitionId));

    return new Petition(nextPetitionId++, createBuilding(buildingType));
}

PetitionManager::~PetitionManager() {
    delete currentPetition;
    for (Petition* petition : underConstructionPetitions)
    {
        delete petition;
    }
}


// Restore helpers

void PetitionManager::restoreUnderConstruction(Petition* petition)
{
    underConstructionPetitions.push_back(petition);
    // Advance nextPetitionId past any restored IDs so new petitions never
    // reuse an ID that already exists in the database.
    if (petition->getId() >= nextPetitionId) {
        nextPetitionId = petition->getId() + 1;
    }
}

void PetitionManager::restoreCurrentPetition(Petition* petition)
{
    // Discard the petition that was auto-generated at construction time.
    delete currentPetition;
    currentPetition = petition;

    if (petition->getId() >= nextPetitionId) {
        nextPetitionId = petition->getId() + 1;
    }
}
