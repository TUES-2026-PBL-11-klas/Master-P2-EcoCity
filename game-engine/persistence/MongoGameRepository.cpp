#include "MongoGameRepository.hpp"

#include <stdexcept>
#include <iostream>

#include <bsoncxx/v_noabi/bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/v_noabi/bsoncxx/types.hpp>
#include <mongocxx/v_noabi/mongocxx/collection.hpp>
#include <mongocxx/v_noabi/mongocxx/cursor.hpp>
#include <mongocxx/v_noabi/mongocxx/database.hpp>
#include <mongocxx/v_noabi/mongocxx/options/replace.hpp>
#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/instance.hpp>
#include <mongocxx/v_noabi/mongocxx/uri.hpp>

#include "../domain/Building.hpp"
#include "../domain/BuildingType.hpp"
#include "../domain/Petition.hpp"
#include "../domain/Resource.hpp"
#include "../domain/ResourceEffect.hpp"
#include "../domain/ResourceType.hpp"

#include "../Logger.hpp"
#include "../Tracer.hpp"
#include "../exceptions/PersistenceException.hpp"

using bsoncxx::builder::basic::document;
using bsoncxx::builder::basic::kvp;

namespace {
std::string toString(ResourceType type)
{
    switch (type) {
    case RESOURCE_UNSPECIFIED: return "RESOURCE_UNSPECIFIED";
    case WATER:                return "WATER";
    case ENERGY:               return "ENERGY";
    case MONEY:                return "MONEY";
    case POPULATION:           return "POPULATION";
    case CO2:                  return "CO2";
    default:                   return "UNKNOWN";
    }
}

std::string toString(BuildingType type)
{
    switch (type) {
    case BUILDING_UNSPECIFIED:          return "BUILDING_UNSPECIFIED";
    case POWER_PLANT:                   return "POWER_PLANT";
    case WATER_TREATMENT_PLANT:         return "WATER_TREATMENT_PLANT";
    case SOLAR_PANEL_FARM:              return "SOLAR_PANEL_FARM";
    case SOLAR_PANEL_ROOFTOPS:          return "SOLAR_PANEL_ROOFTOPS";
    case PUBLIC_TRANSPORT_UPGRADE:      return "PUBLIC_TRANSPORT_UPGRADE";
    case WIND_TURBINE_FARM:             return "WIND_TURBINE_FARM";
    case HYDROELECTRIC_PLANT:           return "HYDROELECTRIC_PLANT";
    case URBAN_GREENING:                return "URBAN_GREENING";
    case WATER_SAVING_INFRASTRUCTURE:   return "WATER_SAVING_INFRASTRUCTURE";
    case INDUSTRIAL_ZONE:               return "INDUSTRIAL_ZONE";
    case AIRPORT_EXPANSION:             return "AIRPORT_EXPANSION";
    case ROAD_IMPROVEMENT:              return "ROAD_IMPROVEMENT";
    default:                            return "UNKNOWN";
    }
}

ResourceType resourceTypeFromString(const std::string& s)
{
    if (s == "WATER")      return WATER;
    if (s == "ENERGY")     return ENERGY;
    if (s == "MONEY")      return MONEY;
    if (s == "POPULATION") return POPULATION;
    if (s == "CO2")        return CO2;
    // An unrecognised string means the DB contains a value this code does not
    // know about -- that is a data-integrity problem worth surfacing immediately.
    throw std::invalid_argument("Unknown ResourceType string: \"" + s + "\"");
}

BuildingType buildingTypeFromString(const std::string& s)
{
    if (s == "POWER_PLANT")                  return POWER_PLANT;
    if (s == "WATER_TREATMENT_PLANT")        return WATER_TREATMENT_PLANT;
    if (s == "SOLAR_PANEL_FARM")             return SOLAR_PANEL_FARM;
    if (s == "SOLAR_PANEL_ROOFTOPS")         return SOLAR_PANEL_ROOFTOPS;
    if (s == "PUBLIC_TRANSPORT_UPGRADE")     return PUBLIC_TRANSPORT_UPGRADE;
    if (s == "WIND_TURBINE_FARM")            return WIND_TURBINE_FARM;
    if (s == "HYDROELECTRIC_PLANT")          return HYDROELECTRIC_PLANT;
    if (s == "URBAN_GREENING")               return URBAN_GREENING;
    if (s == "WATER_SAVING_INFRASTRUCTURE")  return WATER_SAVING_INFRASTRUCTURE;
    if (s == "INDUSTRIAL_ZONE")              return INDUSTRIAL_ZONE;
    if (s == "AIRPORT_EXPANSION")            return AIRPORT_EXPANSION;
    if (s == "ROAD_IMPROVEMENT")             return ROAD_IMPROVEMENT;
    throw std::invalid_argument("Unknown BuildingType string: \"" + s + "\"");
}

double getDeltaForResource(const std::vector<ResourceEffect>& effects, ResourceType type)
{
    for (const ResourceEffect& effect : effects) {
        if (effect.type == type) {
            return static_cast<double>(effect.deltaValue);
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
        kvp("cost", static_cast<double>(building->getBuildCost())),
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


// Safely read a string field from a BSON document view.
std::string getString(const bsoncxx::document::view& doc, const std::string& key)
{
    auto it = doc.find(key);
    if (it == doc.end()) return "";
    if (it->type() != bsoncxx::type::k_string) return "";
    return std::string(it->get_string().value);
}

// Safely read a numeric field as long long (handles both double and int32/64).
long long getNumber(const bsoncxx::document::view& doc, const std::string& key)
{
    auto it = doc.find(key);
    if (it == doc.end()) return 0;

    if (it->type() == bsoncxx::type::k_int64) return it->get_int64().value;
    if (it->type() == bsoncxx::type::k_int32) return it->get_int32().value;
    if (it->type() == bsoncxx::type::k_double) return static_cast<long long>(it->get_double().value);

    return 0;
}

// Read one petition row into a SavedGame::PetitionData.
SavedGame::PetitionData readPetitionDoc(const bsoncxx::document::view& doc)
{
    SavedGame::PetitionData p;
    p.id            = static_cast<int>(getNumber(doc, "petition_id"));
    p.buildingType  = buildingTypeFromString(getString(doc, "type"));
    p.cost          = getNumber(doc, "cost");
    p.ticksRemaining = static_cast<int>(getNumber(doc, "ticks_needed_for_construction"));
    return p;
}

}

MongoGameRepository::MongoGameRepository(const std::string& connectionString, const std::string& databaseName)
    : client(mongocxx::uri{connectionString}), databaseName(databaseName) {}

void MongoGameRepository::saveGame(
    const std::string& gameId,
    const ResourceManager& resourceManager,
    const PetitionManager& petitionManager,
    const City& city)
{
    TRACE("MongoGameRepository", "saveGame");

    try {
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

    LOG_INFO("MongoGameRepository", "game_saved", "game_id=" + gameId);
    } catch (const std::exception& e) {
        // Wrap any MongoDB or BSON exception in PersistenceException so callers
        // do not need to know about mongocxx internals.
        throw PersistenceException(PersistenceException::Operation::SAVE, e.what());
    }
}

SavedGame MongoGameRepository::loadGame(const std::string& gameId)
{
    TRACE("MongoGameRepository", "loadGame");

    try {
    SavedGame result{};
    result.found = false;

    mongocxx::database db = client[databaseName];

    // check the game exists
    auto gameDoc = db["games"].find_one(buildGameIdFilter(gameId).extract());
    if (!gameDoc) {
        std::cerr << "[Load] No save found for game_id=" << gameId << "\n";
        return result;
    }
    result.found = true;

    // resources
    {
        auto cursor = db["resources"].find(buildGameIdFilter(gameId).extract());
        for (const auto& doc : cursor) {
            SavedGame::ResourceData rd;
            rd.type          = resourceTypeFromString(getString(doc, "type"));
            rd.amount        = getNumber(doc, "amount");
            rd.changesPerTick = getNumber(doc, "changes_per_tick");
            result.resources.push_back(rd);
        }
    }

    // current_petition
    {
        auto doc = db["current_petition"].find_one(buildGameIdFilter(gameId).extract());
        if (doc) {
            result.hasCurrentPetition = true;
            result.currentPetition    = readPetitionDoc(doc->view());
        }
    }

    // active_petitions (under construction)
    {
        auto cursor = db["active_petitions"].find(buildGameIdFilter(gameId).extract());
        for (const auto& doc : cursor) {
            result.underConstructionPetitions.push_back(readPetitionDoc(doc));
        }
    }

    // petition_counts (building tallies)
    {
        auto cursor = db["petition_counts"].find(buildGameIdFilter(gameId).extract());
        for (const auto& doc : cursor) {
            BuildingType bt = buildingTypeFromString(getString(doc, "type"));
            int count       = static_cast<int>(getNumber(doc, "count"));
            result.buildingCounts[bt] = count;
        }
    }

    std::cout << "[Load] Game loaded: "
              << result.resources.size()           << " resources, "
              << result.buildingCounts.size()       << " building types, "
              << result.underConstructionPetitions.size() << " buildings under construction, "
              << "current petition: " << (result.hasCurrentPetition ? "yes" : "none")
              << "\n";

    LOG_INFO("MongoGameRepository", "game_loaded", "game_id=" + gameId);

    return result;
    } catch (const std::exception& e) {
        throw PersistenceException(PersistenceException::Operation::LOAD, e.what());
    }
}