// Integration tests for MongoGameRepository
//
// These tests require a live MongoDB instance. The connection URI is read from the environment variable MONGO_TEST_URI (currently "mongodb://localhost:27017")
// A dedicated database named "eco_city_test" is used and wiped before/after each test so the suite is safe to run repeatedly

#include <gtest/gtest.h>

#include "../domain/buildings/BuildingFactory.hpp"
#include "../persistence/MongoGameRepository.hpp"
#include "../services/PetitionManager.hpp"
#include "../services/ResourceManager.hpp"
#include "../domain/City.hpp"

#include <cstdlib>
#include <string>

#include <mongocxx/instance.hpp>

static mongocxx::instance instance{};

namespace {

std::string mongoUri()
{
    const char* env = std::getenv("MONGO_TEST_URI");
    return env ? std::string(env) : "mongodb://localhost:27017";
}

constexpr const char* TEST_DB   = "eco_city_test";
constexpr const char* TEST_GAME = "integration-test-game";

// Creates the repository once per test and drops all test collections in SetUp so each test starts from a clean slate
class IntegrationMongoTest : public ::testing::Test {
    protected:
        MongoGameRepository repo{mongoUri(), TEST_DB};

        void SetUp() override
        {
            // Wipe every collection that saveGame/loadGame touches.
            ResourceManager rm;
            PetitionManager pm;
            City city;

            // Saving with no data is the cheapest way to create + clear the game record before the real test data is written
            repo.saveGame(TEST_GAME, rm, pm, city);
        }
};

} // namespace end

// Round-trip: resources
TEST_F(IntegrationMongoTest, SaveAndLoadResourcesRoundTrip) {
    ResourceManager rm;
    PetitionManager pm;
    City city;

    // Give each resource a distinctive value and delta so we can tell them apart
    rm.changeResourceValue(WATER,      500'000LL - rm.getResourceValue(WATER));
    rm.changeResourceValue(ENERGY,   1'234'567LL - rm.getResourceValue(ENERGY));
    rm.changeResourceValue(MONEY,       99'000LL - rm.getResourceValue(MONEY));
    rm.changeResourceValue(POPULATION, 750'000LL - rm.getResourceValue(POPULATION));
    rm.changeResourceValue(CO2,      2'000'000LL - rm.getResourceValue(CO2));

    rm.setDeltaForResourceType(WATER,      -5'000LL);
    rm.setDeltaForResourceType(ENERGY,    -12'000LL);
    rm.setDeltaForResourceType(MONEY,      +8'000LL);
    rm.setDeltaForResourceType(POPULATION, +3'500LL);
    rm.setDeltaForResourceType(CO2,        +7'777LL);

    repo.saveGame(TEST_GAME, rm, pm, city);
    SavedGame loaded = repo.loadGame(TEST_GAME);

    ASSERT_TRUE(loaded.found);
    ASSERT_EQ(loaded.resources.size(), 5u);

    // Build a lookup map from the loaded vector
    std::unordered_map<ResourceType, SavedGame::ResourceData> byType;
    for (const auto& rd : loaded.resources) {
        byType[rd.type] = rd;
    }

    EXPECT_EQ(byType[WATER].amount,       500'000LL);
    EXPECT_EQ(byType[WATER].changesPerTick, -5'000LL);

    EXPECT_EQ(byType[ENERGY].amount,      1'234'567LL);
    EXPECT_EQ(byType[ENERGY].changesPerTick, -12'000LL);

    EXPECT_EQ(byType[MONEY].amount,        99'000LL);
    EXPECT_EQ(byType[MONEY].changesPerTick, +8'000LL);

    EXPECT_EQ(byType[POPULATION].amount,  750'000LL);
    EXPECT_EQ(byType[POPULATION].changesPerTick, +3'500LL);

    EXPECT_EQ(byType[CO2].amount,       2'000'000LL);
    EXPECT_EQ(byType[CO2].changesPerTick, +7'777LL);
}

// Round-trip: under-construction petitions
TEST_F(IntegrationMongoTest, SaveAndLoadUnderConstructionPetitions) {
    ResourceManager rm;
    PetitionManager pm;
    City city;

    // Put two petitions under construction with known IDs and building types
    pm.restoreUnderConstruction(
        new Petition(10, createBuilding(SOLAR_PANEL_FARM)));
    pm.restoreUnderConstruction(
        new Petition(11, createBuilding(WIND_TURBINE_FARM)));

    repo.saveGame(TEST_GAME, rm, pm, city);
    SavedGame loaded = repo.loadGame(TEST_GAME);

    ASSERT_TRUE(loaded.found);
    ASSERT_EQ(loaded.underConstructionPetitions.size(), 2u);

    // Order from MongoDB is not guaranteed, so search by id
    auto findById = [&](int id) -> const SavedGame::PetitionData* {
        for (const auto& p : loaded.underConstructionPetitions) {
            if (p.id == id) return &p;
        }
        return nullptr;
    };

    const SavedGame::PetitionData* p10 = findById(10);
    const SavedGame::PetitionData* p11 = findById(11);

    ASSERT_NE(p10, nullptr);
    EXPECT_EQ(p10->buildingType, SOLAR_PANEL_FARM);

    ASSERT_NE(p11, nullptr);
    EXPECT_EQ(p11->buildingType, WIND_TURBINE_FARM);
}

// Round-trip: current petition
TEST_F(IntegrationMongoTest, SaveAndLoadCurrentPetition) {
    ResourceManager rm;
    PetitionManager pm;
    City city;

    // Replace the auto-generated petition with a known one
    pm.restoreCurrentPetition(new Petition(42, createBuilding(POWER_PLANT)));

    repo.saveGame(TEST_GAME, rm, pm, city);
    SavedGame loaded = repo.loadGame(TEST_GAME);

    ASSERT_TRUE(loaded.found);
    ASSERT_TRUE(loaded.hasCurrentPetition);
    EXPECT_EQ(loaded.currentPetition.id, 42);
    EXPECT_EQ(loaded.currentPetition.buildingType, POWER_PLANT);
}

// Round-trip: building counts (City)
TEST_F(IntegrationMongoTest, SaveAndLoadBuildingCounts) {
    ResourceManager rm;
    PetitionManager pm;
    City city;

    city.addBuilding(HYDROELECTRIC_PLANT);
    city.addBuilding(HYDROELECTRIC_PLANT);
    city.addBuilding(URBAN_GREENING);

    repo.saveGame(TEST_GAME, rm, pm, city);
    SavedGame loaded = repo.loadGame(TEST_GAME);

    ASSERT_TRUE(loaded.found);
    EXPECT_EQ(loaded.buildingCounts[HYDROELECTRIC_PLANT], 2);
    EXPECT_EQ(loaded.buildingCounts[URBAN_GREENING],      1);
    EXPECT_EQ(loaded.buildingCounts[POWER_PLANT],         0);
}

// Load non-existent game
TEST_F(IntegrationMongoTest, LoadNonExistentGameReturnsFalse) {
    SavedGame loaded = repo.loadGame("game-that-does-not-exist-xyz");

    EXPECT_FALSE(loaded.found);
}

// Overwrite: second save replaces first
TEST_F(IntegrationMongoTest, SecondSaveOverwritesPreviousData) {
    ResourceManager rm1;
    PetitionManager pm1;
    City city1;
    rm1.changeResourceValue(WATER, 111'111LL - rm1.getResourceValue(WATER));
    repo.saveGame(TEST_GAME, rm1, pm1, city1);

    // Second save with different values
    ResourceManager rm2;
    PetitionManager pm2;
    City city2;
    rm2.changeResourceValue(WATER, 999'999LL - rm2.getResourceValue(WATER));
    repo.saveGame(TEST_GAME, rm2, pm2, city2);

    SavedGame loaded = repo.loadGame(TEST_GAME);

    ASSERT_TRUE(loaded.found);
    auto it = std::find_if(loaded.resources.begin(), loaded.resources.end(),
        [](const SavedGame::ResourceData& rd) { return rd.type == WATER; });
    ASSERT_NE(it, loaded.resources.end());
    EXPECT_EQ(it->amount, 999'999LL);
}
