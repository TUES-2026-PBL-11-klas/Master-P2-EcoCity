// =============================================================================
// GameServiceTest.cpp
// Google Test suite for GameService and City.
//
// Dependencies not uploaded (IGameService, ResourceManager, PetitionManager
// and their transitive deps) are stubbed here so the file is self-contained.
//
// Key testing strategy for GameService:
//   - tick() has a side-effect of printing to stdout; we redirect cout to a
//     null-buffer in the fixture so tests stay silent.
//   - checkGameOver() is private, but its result is the return value of tick().
//     We drive it entirely through ResourceManager state.
//   - PetitionManager::tick() returns CompletedConstructions; a MockPetitionManager
//     lets us inject specific completions to verify the apply/add flow.
// =============================================================================

#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================
//  Shared primitive types
// ============================================================

typedef long long int LLint;

constexpr LLint MAX_RESOURCE_VALUE = 1'000'000'000LL;
constexpr LLint MAX_CO2            = 100'000'000LL;

enum ResourceType {
    RESOURCE_UNSPECIFIED,
    WATER, ENERGY, MONEY, POPULATION, CO2
};

enum BuildingType {
    BUILDING_UNSPECIFIED,
    POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM,
    SOLAR_PANEL_ROOFTOPS, PUBLIC_TRANSPORT_UPGRADE, WIND_TURBINE_FARM,
    HYDROELECTRIC_PLANT, URBAN_GREENING, WATER_SAVING_INFRASTRUCTURE,
    INDUSTRIAL_ZONE, AIRPORT_EXPANSION, ROAD_IMPROVEMENT
};

struct ResourceEffect {
    ResourceType type;
    LLint        deltaValue;
};

struct CompletedConstruction {
    BuildingType              type;
    std::vector<ResourceEffect> effects;
};

// ============================================================
//  Resource (minimal, from previous files)
// ============================================================

class Resource {
    ResourceType type;
    LLint        currentValue;
    LLint        deltaPerTick;
public:
    Resource(ResourceType t, LLint v, LLint d)
        : type(t), currentValue(v), deltaPerTick(d) {}

    ResourceType getType()         const { return type; }
    LLint        getCurrentValue() const { return currentValue; }
    LLint        getDeltaValue()   const { return deltaPerTick; }

    void changeDeltaPerTick(LLint d)  { deltaPerTick += d; }

    void changeCurrentValue(LLint d) {
        currentValue += d;
        if (currentValue < 0)                  currentValue = 0;
        if (currentValue > MAX_RESOURCE_VALUE) currentValue = MAX_RESOURCE_VALUE;
    }

    void changeCurrentValue() {
        currentValue += deltaPerTick;
        if (currentValue < 0)                  currentValue = 0;
        if (currentValue > MAX_RESOURCE_VALUE) currentValue = MAX_RESOURCE_VALUE;
    }

    // Test-only setter to directly force a value
    void setCurrentValue(LLint v) { currentValue = v; }
};

// ============================================================
//  ResourceManager stub
//  Exposes setResourceValue() for easy test setup.
// ============================================================

class ResourceManager {
    std::array<Resource, 5> resources;

    int indexOf(ResourceType t) const { return static_cast<int>(t) - 1; }

public:
    using ResourceArray = std::array<Resource, 5>;

    ResourceManager()
        : resources{
            Resource(WATER,      1'000'000LL, 0),
            Resource(ENERGY,     5'000'000LL, 0),
            Resource(MONEY,        250'000LL, 0),
            Resource(POPULATION, 1'000'000LL, 0),
            Resource(CO2,        1'000'000LL, 0)
          } {}

    // Iterators (needed by GameService::checkGameOver)
    ResourceArray::iterator       begin()       { return resources.begin(); }
    ResourceArray::iterator       end()         { return resources.end();   }
    ResourceArray::const_iterator begin() const { return resources.begin(); }
    ResourceArray::const_iterator end()   const { return resources.end();   }

    void tick() { for (Resource& r : resources) r.changeCurrentValue(); }

    void applyEffect(const std::vector<ResourceEffect>& effects) {
        for (const ResourceEffect& e : effects) {
            if (e.type == RESOURCE_UNSPECIFIED) continue;
            resources[indexOf(e.type)].changeDeltaPerTick(e.deltaValue);
        }
    }

    LLint getResourceValue(ResourceType t) const {
        return resources[indexOf(t)].getCurrentValue();
    }

    bool canAfford(LLint amount) const { return getResourceValue(MONEY) >= amount; }

    void changeResourceValue(ResourceType t, LLint delta) {
        if (t == RESOURCE_UNSPECIFIED) return;
        resources[indexOf(t)].changeCurrentValue(delta);
    }

    // Test helper: directly set a resource to a specific value
    void setResourceValue(ResourceType t, LLint value) {
        resources[indexOf(t)].setCurrentValue(value);
    }

    const ResourceArray& getResources() const { return resources; }
};

// ============================================================
//  City (production code, verbatim)
// ============================================================

class City {
    std::unordered_map<BuildingType, int> buildings;
public:
    City() {
        for (auto bt : {BUILDING_UNSPECIFIED, POWER_PLANT, WATER_TREATMENT_PLANT,
                        SOLAR_PANEL_FARM, SOLAR_PANEL_ROOFTOPS, PUBLIC_TRANSPORT_UPGRADE,
                        WIND_TURBINE_FARM, HYDROELECTRIC_PLANT, URBAN_GREENING,
                        WATER_SAVING_INFRASTRUCTURE, INDUSTRIAL_ZONE,
                        AIRPORT_EXPANSION, ROAD_IMPROVEMENT})
            buildings[bt] = 0;
    }

    void addBuilding(BuildingType t)          { buildings[t]++; }
    int  getBuildingCount(BuildingType t) const { return buildings.at(t); }
    const std::unordered_map<BuildingType, int>& getBuildings() const { return buildings; }
};

// ============================================================
//  MockPetitionManager
//  completionsToReturn is consumed one entry per tick() call.
// ============================================================

class PetitionManager {
public:
    std::vector<std::vector<CompletedConstruction>> completionsQueue;
    int tickCallCount = 0;

    std::vector<CompletedConstruction> tick() {
        ++tickCallCount;
        if (completionsQueue.empty()) return {};
        auto result = std::move(completionsQueue.front());
        completionsQueue.erase(completionsQueue.begin());
        return result;
    }

    // Stub methods so GameService compiles if it ever calls these
    void acceptPetition() {}
    void rejectPetition() {}
};

// ============================================================
//  IGameService interface
// ============================================================

class IGameService {
public:
    virtual ~IGameService() = default;
    virtual bool tick()            = 0;
    virtual void readPlayerInput() = 0;
};

// ============================================================
//  GameService (production code, verbatim logic)
//  stdout redirect is handled in the fixture, not here.
// ============================================================

#include <iostream>

namespace {
void printResourceSnapshot(const ResourceManager& rm) {
    std::cout << " | Money: "      << rm.getResourceValue(MONEY)
              << " | Energy: "     << rm.getResourceValue(ENERGY)
              << " | Water: "      << rm.getResourceValue(WATER)
              << " | CO2: "        << rm.getResourceValue(CO2)
              << " | Population: " << rm.getResourceValue(POPULATION)
              << '\n';
}
}

class GameService : public IGameService {
    ResourceManager*  resourceManager;
    PetitionManager*  petitionManager;
    City*             city;

    bool checkGameOver() {
        for (const Resource& r : *resourceManager) {
            if (r.getCurrentValue() <= 0 && r.getType() != CO2)
                return true;
        }
        return resourceManager->getResourceValue(CO2) >= MAX_CO2;
    }

public:
    GameService(ResourceManager* rm, PetitionManager* pm, City* c)
        : resourceManager(rm), petitionManager(pm), city(c) {}

    bool tick() override {
        readPlayerInput();

        const auto completedConstructions = petitionManager->tick();
        for (const CompletedConstruction& cc : completedConstructions) {
            resourceManager->applyEffect(cc.effects);
            city->addBuilding(cc.type);
        }

        resourceManager->tick();
        printResourceSnapshot(*resourceManager);
        return checkGameOver();
    }

    void readPlayerInput() override { /* placeholder */ }
};

// ================================================================
//  Test fixture – silences stdout during each test
// ================================================================

class GameServiceTest : public ::testing::Test {
protected:
    ResourceManager  rm;
    PetitionManager  pm;
    City             city;
    GameService*     gs = nullptr;
    std::streambuf*  originalCoutBuf = nullptr;
    std::ostringstream sink;

    void SetUp() override {
        // Redirect cout so printResourceSnapshot doesn't pollute test output
        originalCoutBuf = std::cout.rdbuf(sink.rdbuf());
        gs = new GameService(&rm, &pm, &city);
    }

    void TearDown() override {
        std::cout.rdbuf(originalCoutBuf);
        delete gs;
    }
};

// ================================================================
//  City – unit tests
// ================================================================

TEST(CityTest, AllBuildingCountsInitiallyZero) {
    City c;
    for (auto bt : {POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM,
                    SOLAR_PANEL_ROOFTOPS, PUBLIC_TRANSPORT_UPGRADE, WIND_TURBINE_FARM,
                    HYDROELECTRIC_PLANT, URBAN_GREENING, WATER_SAVING_INFRASTRUCTURE,
                    INDUSTRIAL_ZONE, AIRPORT_EXPANSION, ROAD_IMPROVEMENT})
        EXPECT_EQ(c.getBuildingCount(bt), 0) << "Expected 0 for type " << bt;
}

TEST(CityTest, AddBuildingIncrementsByOne) {
    City c;
    c.addBuilding(POWER_PLANT);
    EXPECT_EQ(c.getBuildingCount(POWER_PLANT), 1);
}

TEST(CityTest, AddSameBuildingMultipleTimes) {
    City c;
    c.addBuilding(SOLAR_PANEL_FARM);
    c.addBuilding(SOLAR_PANEL_FARM);
    c.addBuilding(SOLAR_PANEL_FARM);
    EXPECT_EQ(c.getBuildingCount(SOLAR_PANEL_FARM), 3);
}

TEST(CityTest, AddDifferentBuildingsAreIndependent) {
    City c;
    c.addBuilding(WIND_TURBINE_FARM);
    c.addBuilding(HYDROELECTRIC_PLANT);
    c.addBuilding(HYDROELECTRIC_PLANT);
    EXPECT_EQ(c.getBuildingCount(WIND_TURBINE_FARM),  1);
    EXPECT_EQ(c.getBuildingCount(HYDROELECTRIC_PLANT), 2);
    EXPECT_EQ(c.getBuildingCount(ROAD_IMPROVEMENT),    0);
}

TEST(CityTest, GetBuildingsReturnsAllEntries) {
    City c;
    const auto& map = c.getBuildings();
    // All 13 BuildingType values should be present (including UNSPECIFIED)
    EXPECT_EQ(map.size(), 13u);
}

TEST(CityTest, GetBuildingsReflectsAddBuilding) {
    City c;
    c.addBuilding(INDUSTRIAL_ZONE);
    EXPECT_EQ(c.getBuildings().at(INDUSTRIAL_ZONE), 1);
}

// ================================================================
//  GameService – tick delegates to PetitionManager
// ================================================================

TEST_F(GameServiceTest, TickCallsPetitionManagerTick) {
    gs->tick();
    EXPECT_EQ(pm.tickCallCount, 1);
}

TEST_F(GameServiceTest, MultipleTicksCallPetitionManagerEachTime) {
    gs->tick();
    gs->tick();
    gs->tick();
    EXPECT_EQ(pm.tickCallCount, 3);
}

// ================================================================
//  GameService – completed constructions propagate to City and ResourceManager
// ================================================================

TEST_F(GameServiceTest, CompletedConstructionAddsBuildingtToCity) {
    pm.completionsQueue.push_back({{POWER_PLANT, {{ENERGY, 500}}}});
    gs->tick();
    EXPECT_EQ(city.getBuildingCount(POWER_PLANT), 1);
}

TEST_F(GameServiceTest, CompletedConstructionAppliesEffectsAsDeltas) {
    pm.completionsQueue.push_back({{WATER_TREATMENT_PLANT, {{WATER, 200}}}});
    LLint waterBefore = rm.getResourceValue(WATER);
    gs->tick(); // applyEffect sets delta=200, then resourceManager->tick() fires it
    EXPECT_EQ(rm.getResourceValue(WATER), waterBefore + 200);
}

TEST_F(GameServiceTest, MultipleCompletionsInOneTick) {
    pm.completionsQueue.push_back({
        {SOLAR_PANEL_FARM,  {{ENERGY,  100}}},
        {WIND_TURBINE_FARM, {{ENERGY,  150}}}
    });
    LLint energyBefore = rm.getResourceValue(ENERGY);
    gs->tick();
    EXPECT_EQ(city.getBuildingCount(SOLAR_PANEL_FARM),  1);
    EXPECT_EQ(city.getBuildingCount(WIND_TURBINE_FARM), 1);
    EXPECT_EQ(rm.getResourceValue(ENERGY), energyBefore + 250);
}

TEST_F(GameServiceTest, NoCompletionsLeaveCityUnchanged) {
    gs->tick();
    for (auto bt : {POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM})
        EXPECT_EQ(city.getBuildingCount(bt), 0);
}

TEST_F(GameServiceTest, CompletionsAccumulateOverMultipleTicks) {
    pm.completionsQueue.push_back({{URBAN_GREENING, {}}});
    pm.completionsQueue.push_back({{URBAN_GREENING, {}}});
    gs->tick();
    gs->tick();
    EXPECT_EQ(city.getBuildingCount(URBAN_GREENING), 2);
}

// ================================================================
//  GameService – resourceManager->tick() is called each game tick
// ================================================================

TEST_F(GameServiceTest, ResourceManagerTickedEachGameTick) {
    rm.applyEffect({{MONEY, 1000}});
    LLint before = rm.getResourceValue(MONEY);
    gs->tick();
    EXPECT_EQ(rm.getResourceValue(MONEY), before + 1000);
}

// ================================================================
//  GameService – checkGameOver via tick() return value
// ================================================================

// -- Normal state: no game over --

TEST_F(GameServiceTest, NoGameOverWithHealthyResources) {
    EXPECT_FALSE(gs->tick());
}

// -- CO2 at maximum triggers game over --

TEST_F(GameServiceTest, GameOverWhenCO2ReachesMax) {
    rm.setResourceValue(CO2, MAX_CO2);
    EXPECT_TRUE(gs->tick());
}

TEST_F(GameServiceTest, GameOverWhenCO2ExceedsMax) {
    rm.setResourceValue(CO2, MAX_CO2 + 1);
    EXPECT_TRUE(gs->tick());
}

TEST_F(GameServiceTest, NoGameOverWhenCO2JustBelowMax) {
    rm.setResourceValue(CO2, MAX_CO2 - 1);
    EXPECT_FALSE(gs->tick());
}

// CO2 at zero should NOT trigger game over (CO2 depletion is fine)
TEST_F(GameServiceTest, CO2DepletedIsNotGameOver) {
    rm.setResourceValue(CO2, 0);
    EXPECT_FALSE(gs->tick());
}

// -- Each non-CO2 resource at zero triggers game over --

TEST_F(GameServiceTest, GameOverWhenWaterDepleted) {
    rm.setResourceValue(WATER, 0);
    EXPECT_TRUE(gs->tick());
}

TEST_F(GameServiceTest, GameOverWhenEnergyDepleted) {
    rm.setResourceValue(ENERGY, 0);
    EXPECT_TRUE(gs->tick());
}

TEST_F(GameServiceTest, GameOverWhenMoneyDepleted) {
    rm.setResourceValue(MONEY, 0);
    EXPECT_TRUE(gs->tick());
}

TEST_F(GameServiceTest, GameOverWhenPopulationDepleted) {
    rm.setResourceValue(POPULATION, 0);
    EXPECT_TRUE(gs->tick());
}

// -- One unit above zero should not trigger game over --

TEST_F(GameServiceTest, NoGameOverWhenResourcesAtOne) {
    rm.setResourceValue(WATER,      1);
    rm.setResourceValue(ENERGY,     1);
    rm.setResourceValue(MONEY,      1);
    rm.setResourceValue(POPULATION, 1);
    rm.setResourceValue(CO2,        1);
    EXPECT_FALSE(gs->tick());
}

// -- Delta-driven depletion: resource ticked down to zero mid-game --

TEST_F(GameServiceTest, GameOverWhenResourceTicksDownToZero) {
    // Set MONEY to 10, apply a delta of -10 so tick() drives it to 0
    rm.setResourceValue(MONEY, 10);
    rm.applyEffect({{MONEY, -10}});
    EXPECT_TRUE(gs->tick()); // after tick, MONEY == 0 → game over
}

TEST_F(GameServiceTest, NoGameOverBeforeResourceReachesZero) {
    rm.setResourceValue(MONEY, 100);
    rm.applyEffect({{MONEY, -10}});
    EXPECT_FALSE(gs->tick()); // MONEY goes to 90, not 0
}

// -- Both a depleted resource AND max CO2: still game over --

TEST_F(GameServiceTest, GameOverWhenBothConditionsMet) {
    rm.setResourceValue(CO2,   MAX_CO2);
    rm.setResourceValue(WATER, 0);
    EXPECT_TRUE(gs->tick());
}

// ================================================================
//  main
// ================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}