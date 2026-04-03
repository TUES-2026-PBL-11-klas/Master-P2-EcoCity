#include "MongoGameRepository.hpp"

#include <stdexcept>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/options/replace.hpp>

#include "domain/Building.hpp"
#include "domain/BuildingType.hpp"
#include "domain/Petition.hpp"
#include "domain/Resource.hpp"
#include "domain/ResourceEffect.hpp"
#include "domain/ResourceType.hpp"

using bsoncxx::builder::basic::document;
using bsoncxx::builder::basic::kvp;

namespace {
std::string toString(ResourceType type)
{
    switch (type) {
    case UNSPECIFIED:
        return "UNSPECIFIED";
    case WATER:
        return "WATER";
    case ENERGY:
        return "ENERGY";
    case MONEY:
        return "MONEY";
    case POPULATION:
        return "POPULATION";
    case CO2:
        return "CO2";
    default:
        return "UNKNOWN";
    }
}

std::string toString(BuildingType type)
{
    switch (type) {
    case BUILDING_UNSPECIFIED:
        return "BUILDING_UNSPECIFIED";
    case POWER_PLANT:
        return "POWER_PLANT";
    case WATER_TREATMENT_PLANT:
        return "WATER_TREATMENT_PLANT";
    case SOLAR_PANEL_FARM:
        return "SOLAR_PANEL_FARM";
    default:
        return "UNKNOWN";
    }
}

double getDeltaForResource(const std::vector<ResourceEffect>& effects, ResourceType type)
{
    for (const ResourceEffect& effect : effects) {
        if (effect.type == type) {
            return effect.deltaValue;
        }
    }

    return 0.0;
}

document buildPetitionDocument(const std::string& gameId, const Petition& petition)
{
    const Building* building = petition.getBuilding();
    if (building == nullptr) {
        throw std::runtime_error("Petition has no building attached");
    }

    const std::vector<ResourceEffect> effects = building->getEffects();
    document petitionDocument{};
    petitionDocument.append(
        kvp("game_id", gameId),
        kvp("petition_id", petition.getId()),
        kvp("type", toString(building->getType())),
        kvp("cost", building->getBuildCost()),
        kvp("ticks_needed_for_construction", building->getTicksToComplete()),
        kvp("water_per_tick_change", getDeltaForResource(effects, WATER)),
        kvp("co2_per_tick_change", getDeltaForResource(effects, CO2)),
        kvp("energy_per_tick_change", getDeltaForResource(effects, ENERGY)),
        kvp("population_per_tick_change", getDeltaForResource(effects, POPULATION)),
        kvp("money_per_tick_change", getDeltaForResource(effects, MONEY))
    );
    return petitionDocument;
}

document buildGameIdFilter(const std::string& gameId)
{
    document filter{};
    filter.append(kvp("game_id", gameId));
    return filter;
}
}

MongoGameRepository::MongoGameRepository(const std::string& connectionString, const std::string& databaseName)
    : instance(), client(mongocxx::uri{connectionString}), databaseName(databaseName) {}

void MongoGameRepository::saveGame(
    const std::string& gameId,
    const ResourceManager& resourceManager,
    const PetitionManager& petitionManager,
    const City& city)
{
    mongocxx::database database = client[databaseName];

    mongocxx::options::replace replaceOptions;
    replaceOptions.upsert(true);

    document gameDocument{};
    gameDocument.append(kvp("game_id", gameId));
    database["games"].replace_one(
        buildGameIdFilter(gameId).extract(),
        gameDocument.extract(),
        replaceOptions
    );

    database["resources"].delete_many(buildGameIdFilter(gameId).extract());
    for (const Resource& resource : resourceManager.getResources()) {
        document resourceDocument{};
        resourceDocument.append(
            kvp("game_id", gameId),
            kvp("type", toString(resource.getType())),
            kvp("amount", static_cast<double>(resource.getCurrentValue())),
            kvp("changes_per_tick", static_cast<double>(resource.getDeltaValue()))
        );
        database["resources"].insert_one(resourceDocument.extract());
    }

    database["active_petitions"].delete_many(buildGameIdFilter(gameId).extract());
    for (const Petition* petition : petitionManager.getUnderConstructionPetitions()) {
        if (petition == nullptr) {
            continue;
        }

        database["active_petitions"].insert_one(buildPetitionDocument(gameId, *petition).extract());
    }

    database["petition_counts"].delete_many(buildGameIdFilter(gameId).extract());
    for (const auto& [buildingType, count] : city.getBuildings()) {
        document petitionCountDocument{};
        petitionCountDocument.append(
            kvp("game_id", gameId),
            kvp("type", toString(buildingType)),
            kvp("count", count)
        );
        database["petition_counts"].insert_one(petitionCountDocument.extract());
    }

    database["current_petition"].delete_many(buildGameIdFilter(gameId).extract());
    if (petitionManager.getCurrentPetition() != nullptr) {
        database["current_petition"].insert_one(
            buildPetitionDocument(gameId, *petitionManager.getCurrentPetition()).extract()
        );
    }
}
