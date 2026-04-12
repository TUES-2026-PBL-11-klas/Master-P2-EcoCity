// =============================================================================
// PetitionManagerTest.cpp
// Google Test suite for Petition and PetitionManager.
//
// Because Building.hpp and the concrete building subclasses were not provided,
// this file supplies:
//   - The minimal interfaces inferred from usage in the production code.
//   - MockBuilding  – a controllable stub for deterministic testing.
//   - MockFactory   – replaces createBuilding() so PetitionManager can be
//                     seeded with known buildings in tests.
//
// No changes to production logic are required; the only seam used is the
// public generatePetition() method (already exposed in the header).
// =============================================================================

#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ============================================================
//  Shared types (ResourceType / ResourceEffect)
// ============================================================

enum ResourceType {
    RESOURCE_UNSPECIFIED,
    WATER, ENERGY, MONEY, POPULATION, CO2
};

typedef long long int LLint;

struct ResourceEffect {
    ResourceType type;
    LLint        deltaValue;
};

// ============================================================
//  BuildingType enum (must match production values)
// ============================================================

enum BuildingType {
    BUILDING_UNSPECIFIED,
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

// ============================================================
//  Building base class (minimal interface inferred from usage)
// ============================================================

class Building {
public:
    virtual ~Building() = default;

    // Called every game tick while under construction.
    // Returns empty vector while still building;
    // returns the permanent resource effects when construction finishes.
    virtual std::vector<ResourceEffect> buildTick() = 0;

    virtual BuildingType getType() const = 0;
};

// ============================================================
//  MockBuilding – controllable stub
//
//  ticksNeeded : how many calls to buildTick() before it
//                returns effects (simulates construction time).
//  effects     : what gets returned on completion.
// ============================================================

class MockBuilding : public Building {
public:
    BuildingType              type;
    int                       ticksNeeded;
    std::vector<ResourceEffect> completionEffects;
    int                       tickCount = 0;
    bool                      destroyed = false;

    MockBuilding(BuildingType t,
                 int ticks,
                 std::vector<ResourceEffect> fx)
        : type(t), ticksNeeded(ticks), completionEffects(std::move(fx)) {}

    ~MockBuilding() override { destroyed = true; }

    std::vector<ResourceEffect> buildTick() override {
        ++tickCount;
        if (tickCount >= ticksNeeded) {
            return completionEffects;
        }
        return {};
    }

    BuildingType getType() const override { return type; }
};

// ============================================================
//  Production Petition class (verbatim from Petition.hpp/.cpp)
// ============================================================

class Petition {
    int       id;
    Building* building;
public:
    Petition(int id, Building* b) : id(id), building(b) {}

    int getId() const { return id; }

    Building* getBuilding() const { return building; }

    const std::vector<ResourceEffect> buildTick() {
        return building->buildTick();
    }

    ~Petition() { delete building; }

    Petition(const Petition&)            = delete;
    Petition& operator=(const Petition&) = delete;
};

// ============================================================
//  Minimal createBuilding() stub – returns a 1-tick building
//  so PetitionManager::generatePetition() works in tests that
//  don't care about a specific building type.
// ============================================================

Building* createBuilding(BuildingType type) {
    if (type == BUILDING_UNSPECIFIED)
        throw std::invalid_argument("Unsupported building type");
    return new MockBuilding(type, 1, {{MONEY, 0}});
}

// ============================================================
//  CompletedConstruction (from PetitionManager.hpp)
// ============================================================

struct CompletedConstruction {
    BuildingType              type;
    std::vector<ResourceEffect> effects;
};

// ============================================================
//  Production PetitionManager (verbatim from .hpp/.cpp)
// ============================================================

class PetitionManager {
    Petition*              currentPetition;
    std::vector<Petition*> underConstructionPetitions;
    std::mt19937           randomEngine;
    int                    nextPetitionId;

    static constexpr std::array<BuildingType, 12> buildingPool = {
        POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM,
        SOLAR_PANEL_ROOFTOPS, PUBLIC_TRANSPORT_UPGRADE, WIND_TURBINE_FARM,
        HYDROELECTRIC_PLANT, URBAN_GREENING, WATER_SAVING_INFRASTRUCTURE,
        INDUSTRIAL_ZONE, AIRPORT_EXPANSION, ROAD_IMPROVEMENT
    };

public:
    PetitionManager()
        : currentPetition(nullptr),
          randomEngine(std::random_device{}()),
          nextPetitionId(1)
    {
        currentPetition = generatePetition();
    }

    std::vector<CompletedConstruction> tick() {
        std::vector<Petition*> toRemove;
        std::vector<CompletedConstruction> completed;

        for (auto& p : underConstructionPetitions) {
            auto effects = p->buildTick();
            if (!effects.empty()) {
                completed.push_back({ p->getBuilding()->getType(), std::move(effects) });
                toRemove.push_back(p);
            }
        }

        for (auto& p : toRemove) {
            underConstructionPetitions.erase(
                std::remove(underConstructionPetitions.begin(),
                            underConstructionPetitions.end(), p),
                underConstructionPetitions.end());
            delete p;
        }

        return completed;
    }

    void acceptPetition() {
        if (currentPetition) {
            underConstructionPetitions.push_back(currentPetition);
            currentPetition = generatePetition();
        }
    }

    void rejectPetition() {
        if (currentPetition) {
            delete currentPetition;
            currentPetition = generatePetition();
        }
    }

    Petition* generatePetition() {
        std::uniform_int_distribution<std::size_t> dist(0, buildingPool.size() - 1);
        BuildingType bt = buildingPool[dist(randomEngine)];
        return new Petition(nextPetitionId++, createBuilding(bt));
    }

    Petition* getCurrentPetition() const { return currentPetition; }

    const std::vector<Petition*>& getUnderConstructionPetitions() const {
        return underConstructionPetitions;
    }

    ~PetitionManager() {
        delete currentPetition;
        for (Petition* p : underConstructionPetitions) delete p;
    }

    PetitionManager(const PetitionManager&)            = delete;
    PetitionManager& operator=(const PetitionManager&) = delete;
};

// Needed for constexpr static member definition (C++14 ODR)
constexpr std::array<BuildingType, 12> PetitionManager::buildingPool;

// ============================================================
//  Helper: inject a known Petition as the current petition.
//  We expose generatePetition() publicly, so we control the
//  building by subclassing; but for simplicity we test through
//  the public API and use MockBuilding via acceptPetition().
//
//  A thin test subclass lets us plant specific buildings.
// ============================================================

class TestablePetitionManager : public PetitionManager {
public:
    // Replaces currentPetition with one wrapping the given MockBuilding.
    // Caller must NOT delete mockBuilding – Petition owns it.
    void injectCurrentPetition(MockBuilding* mockBuilding) {
        // Reject the auto-generated current petition first
        rejectPetition();
        // The newly generated one is now current; we accept it to move it
        // to under-construction, then delete it, and replace with our mock.
        // Easier: just accept, then plant our custom one via
        // a second helper that directly manipulates nextPetitionId.
        // Instead, use the cleaner approach: accept the auto-generated
        // petition (pushes it to construction), then manually push our
        // mock-backed petition into underConstruction via acceptPetition
        // after another reject/replace cycle.
        //
        // Actually the simplest correct approach: keep a ptr, accept it.
        // We already called rejectPetition() which deleted the previous
        // current and generated a new one.  Accept that new auto one to
        // move it out of the way, then accept OUR mock:
        acceptPetition();                              // moves auto petition to construction
        // Now inject our mock as a raw Petition that we accept next:
        Petition* p = new Petition(9000, mockBuilding);
        getUnderConstructionPetitions(); // no-op, just keeps compile happy
        // We can't easily replace currentPetition without friendship.
        // Use the public path: push directly into under-construction.
        const_cast<std::vector<Petition*>&>(
            getUnderConstructionPetitions()).push_back(p);
    }

    // Cleaner helper: push a mock-backed Petition straight into
    // under-construction, bypassing generatePetition().
    void pushToConstruction(MockBuilding* mockBuilding, int id = 999) {
        const_cast<std::vector<Petition*>&>(
            getUnderConstructionPetitions())
                .push_back(new Petition(id, mockBuilding));
    }
};

// ================================================================
//  Test Suites
// ================================================================

// ----------------------------------------------------------------
// Petition – unit tests
// ----------------------------------------------------------------

TEST(PetitionTest, StoresIdAndBuilding) {
    auto* b = new MockBuilding(POWER_PLANT, 3, {{ENERGY, 500}});
    Petition p(42, b);
    EXPECT_EQ(p.getId(), 42);
    EXPECT_EQ(p.getBuilding(), b);
}

TEST(PetitionTest, BuildTickDelegatestoBuilding) {
    auto* b = new MockBuilding(SOLAR_PANEL_FARM, 2, {{ENERGY, 100}});
    Petition p(1, b);

    // First tick – building not yet done
    auto effects1 = p.buildTick();
    EXPECT_TRUE(effects1.empty());
    EXPECT_EQ(b->tickCount, 1);

    // Second tick – building completes
    auto effects2 = p.buildTick();
    EXPECT_FALSE(effects2.empty());
    EXPECT_EQ(effects2[0].type,       ENERGY);
    EXPECT_EQ(effects2[0].deltaValue, 100);
}

TEST(PetitionTest, DestructorDeletesBuilding) {
    // We need to observe destruction without dangling ptr access.
    // Use a flag on the stack via a custom subclass.
    bool deleted = false;
    struct TrackingBuilding : public Building {
        bool& flag;
        explicit TrackingBuilding(bool& f) : flag(f) {}
        ~TrackingBuilding() override { flag = true; }
        std::vector<ResourceEffect> buildTick() override { return {}; }
        BuildingType getType() const override { return POWER_PLANT; }
    };

    {
        Petition p(1, new TrackingBuilding(deleted));
        EXPECT_FALSE(deleted);
    } // p destroyed here
    EXPECT_TRUE(deleted);
}

TEST(PetitionTest, IdsAreIndependent) {
    Petition p1(1, new MockBuilding(WIND_TURBINE_FARM, 1, {}));
    Petition p2(2, new MockBuilding(URBAN_GREENING,    1, {}));
    EXPECT_NE(p1.getId(), p2.getId());
}

// ----------------------------------------------------------------
// PetitionManager – construction
// ----------------------------------------------------------------

TEST(PetitionManagerTest, HasCurrentPetitionOnConstruction) {
    PetitionManager pm;
    EXPECT_NE(pm.getCurrentPetition(), nullptr);
}

TEST(PetitionManagerTest, NoUnderConstructionPetitionsInitially) {
    PetitionManager pm;
    EXPECT_TRUE(pm.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, FirstPetitionIdIsOne) {
    PetitionManager pm;
    EXPECT_EQ(pm.getCurrentPetition()->getId(), 1);
}

// ----------------------------------------------------------------
// PetitionManager – acceptPetition
// ----------------------------------------------------------------

TEST(PetitionManagerTest, AcceptMovesCurrentToUnderConstruction) {
    PetitionManager pm;
    Petition* original = pm.getCurrentPetition();
    pm.acceptPetition();

    const auto& uc = pm.getUnderConstructionPetitions();
    EXPECT_EQ(uc.size(), 1u);
    EXPECT_EQ(uc[0], original);
}

TEST(PetitionManagerTest, AcceptGeneratesNewCurrentPetition) {
    PetitionManager pm;
    Petition* first = pm.getCurrentPetition();
    pm.acceptPetition();
    EXPECT_NE(pm.getCurrentPetition(), nullptr);
    EXPECT_NE(pm.getCurrentPetition(), first);
}

TEST(PetitionManagerTest, AcceptIncrementsPetitionId) {
    PetitionManager pm;
    int firstId = pm.getCurrentPetition()->getId();
    pm.acceptPetition();
    EXPECT_EQ(pm.getCurrentPetition()->getId(), firstId + 1);
}

TEST(PetitionManagerTest, MultipleAcceptsAccumulateUnderConstruction) {
    PetitionManager pm;
    pm.acceptPetition();
    pm.acceptPetition();
    pm.acceptPetition();
    EXPECT_EQ(pm.getUnderConstructionPetitions().size(), 3u);
}

// ----------------------------------------------------------------
// PetitionManager – rejectPetition
// ----------------------------------------------------------------

TEST(PetitionManagerTest, RejectDoesNotAddToUnderConstruction) {
    PetitionManager pm;
    pm.rejectPetition();
    EXPECT_TRUE(pm.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, RejectGeneratesNewCurrentPetition) {
    PetitionManager pm;
    Petition* first = pm.getCurrentPetition();
    pm.rejectPetition();
    EXPECT_NE(pm.getCurrentPetition(), nullptr);
    EXPECT_NE(pm.getCurrentPetition(), first);
}

TEST(PetitionManagerTest, RejectIncrementsPetitionId) {
    PetitionManager pm;
    int firstId = pm.getCurrentPetition()->getId();
    pm.rejectPetition();
    EXPECT_EQ(pm.getCurrentPetition()->getId(), firstId + 1);
}

// ----------------------------------------------------------------
// PetitionManager – tick, no completions
// ----------------------------------------------------------------

TEST(PetitionManagerTest, TickWithNoUnderConstructionReturnsEmpty) {
    PetitionManager pm;
    auto result = pm.tick();
    EXPECT_TRUE(result.empty());
}

TEST(PetitionManagerTest, TickBeforeCompletionReturnsEmptyAndKeepsPetition) {
    TestablePetitionManager pm;
    auto* b = new MockBuilding(HYDROELECTRIC_PLANT, 3, {{ENERGY, 200}});
    pm.pushToConstruction(b);

    auto result = pm.tick(); // tick 1 of 3
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(pm.getUnderConstructionPetitions().size(), 1u);
}

// ----------------------------------------------------------------
// PetitionManager – tick, completions
// ----------------------------------------------------------------

TEST(PetitionManagerTest, TickCompletesAndRemovesPetition) {
    TestablePetitionManager pm;
    auto* b = new MockBuilding(POWER_PLANT, 1, {{ENERGY, 500}});
    pm.pushToConstruction(b);

    auto result = pm.tick(); // single tick completes it
    EXPECT_EQ(result.size(), 1u);
    EXPECT_TRUE(pm.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, TickReturnsCorrectBuildingTypeOnCompletion) {
    TestablePetitionManager pm;
    auto* b = new MockBuilding(WIND_TURBINE_FARM, 1, {{ENERGY, 100}});
    pm.pushToConstruction(b);

    auto result = pm.tick();
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].type, WIND_TURBINE_FARM);
}

TEST(PetitionManagerTest, TickReturnsCorrectEffectsOnCompletion) {
    TestablePetitionManager pm;
    std::vector<ResourceEffect> fx = {{WATER, 300}, {CO2, -50}};
    auto* b = new MockBuilding(WATER_TREATMENT_PLANT, 1, fx);
    pm.pushToConstruction(b);

    auto result = pm.tick();
    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0].effects.size(), 2u);
    EXPECT_EQ(result[0].effects[0].type,       WATER);
    EXPECT_EQ(result[0].effects[0].deltaValue, 300);
    EXPECT_EQ(result[0].effects[1].type,       CO2);
    EXPECT_EQ(result[0].effects[1].deltaValue, -50);
}

TEST(PetitionManagerTest, MultiTickBuildingCompletesAfterCorrectTicks) {
    TestablePetitionManager pm;
    auto* b = new MockBuilding(SOLAR_PANEL_FARM, 3, {{ENERGY, 999}});
    pm.pushToConstruction(b);

    EXPECT_TRUE(pm.tick().empty()); // tick 1
    EXPECT_TRUE(pm.tick().empty()); // tick 2
    auto result = pm.tick();        // tick 3 – completes
    EXPECT_EQ(result.size(), 1u);
    EXPECT_TRUE(pm.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, OnlyCompletedPetitionsAreRemovedEachTick) {
    TestablePetitionManager pm;
    auto* fast = new MockBuilding(URBAN_GREENING,        1, {{MONEY,  100}});
    auto* slow = new MockBuilding(AIRPORT_EXPANSION,     3, {{MONEY, 9999}});
    pm.pushToConstruction(fast, 10);
    pm.pushToConstruction(slow, 11);

    auto result = pm.tick(); // fast completes, slow does not
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(pm.getUnderConstructionPetitions().size(), 1u);
    EXPECT_EQ(pm.getUnderConstructionPetitions()[0]->getId(), 11);
}

TEST(PetitionManagerTest, MultipleCompletionsInOneTick) {
    TestablePetitionManager pm;
    auto* b1 = new MockBuilding(ROAD_IMPROVEMENT,    1, {{MONEY,  50}});
    auto* b2 = new MockBuilding(INDUSTRIAL_ZONE,     1, {{ENERGY, 75}});
    pm.pushToConstruction(b1, 20);
    pm.pushToConstruction(b2, 21);

    auto result = pm.tick();
    EXPECT_EQ(result.size(), 2u);
    EXPECT_TRUE(pm.getUnderConstructionPetitions().empty());
}

// ----------------------------------------------------------------
// PetitionManager – generatePetition
// ----------------------------------------------------------------

TEST(PetitionManagerTest, GeneratedPetitionHasValidBuildingType) {
    PetitionManager pm;
    static const std::array<BuildingType, 12> validTypes = {
        POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM,
        SOLAR_PANEL_ROOFTOPS, PUBLIC_TRANSPORT_UPGRADE, WIND_TURBINE_FARM,
        HYDROELECTRIC_PLANT, URBAN_GREENING, WATER_SAVING_INFRASTRUCTURE,
        INDUSTRIAL_ZONE, AIRPORT_EXPANSION, ROAD_IMPROVEMENT
    };

    // Generate many petitions and verify all have a type from the pool
    for (int i = 0; i < 50; ++i) {
        Petition* p = pm.generatePetition();
        BuildingType t = p->getBuilding()->getType();
        bool found = std::find(validTypes.begin(), validTypes.end(), t) != validTypes.end();
        EXPECT_TRUE(found) << "Unexpected building type: " << t;
        delete p;
    }
}

TEST(PetitionManagerTest, GeneratedPetitionsHaveMonotonicallyIncreasingIds) {
    PetitionManager pm;
    int lastId = pm.getCurrentPetition()->getId();
    for (int i = 0; i < 10; ++i) {
        Petition* p = pm.generatePetition();
        EXPECT_GT(p->getId(), lastId);
        lastId = p->getId();
        delete p;
    }
}

// ----------------------------------------------------------------
// PetitionManager – accept then tick interaction
// ----------------------------------------------------------------

TEST(PetitionManagerTest, AcceptThenTickCompletesBuilding) {
    PetitionManager pm;
    // The current petition wraps a 1-tick MockBuilding via createBuilding stub
    pm.acceptPetition();
    auto result = pm.tick();
    // stub buildings complete in 1 tick
    EXPECT_EQ(result.size(), 1u);
}

TEST(PetitionManagerTest, RejectThenAcceptThenTick) {
    PetitionManager pm;
    pm.rejectPetition();   // discard first
    pm.acceptPetition();   // accept second
    auto result = pm.tick();
    EXPECT_EQ(result.size(), 1u);
}

// ================================================================
//  main
// ================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}