#include <gtest/gtest.h>

#include "../domain/Petition.hpp"
#include "../domain/buildings/BuildingFactory.hpp"
#include "../services/PetitionManager.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

class StubBuilding : public Building {
    public:
        StubBuilding(BuildingType type, int ticksToComplete, std::vector<ResourceEffect> configuredEffects, LLint buildCost = 0)
            : Building(type, buildCost, ticksToComplete), configuredEffects(std::move(configuredEffects))
        {
            effects = this->configuredEffects;
        }

    private:
        std::vector<ResourceEffect> configuredEffects;

        std::vector<ResourceEffect> Effects() const override
        {
            return configuredEffects;
        }
};

class TrackingBuilding : public Building {
    public:
        explicit TrackingBuilding(bool& deletedFlag)
            : Building(POWER_PLANT, 0, 1), deletedFlag(deletedFlag)
        {
            effects = Effects();
        }

        ~TrackingBuilding() override
        {
            deletedFlag = true;
        }

    private:
        bool& deletedFlag;

        std::vector<ResourceEffect> Effects() const override
        {
            return {};
        }
};

Petition* makePetition(int id, BuildingType type, int ticksToComplete, std::vector<ResourceEffect> effects)
{
    return new Petition(id, new StubBuilding(type, ticksToComplete, std::move(effects)));
}

}

TEST(PetitionTest, StoresIdAndBuilding) {
    auto* building = new StubBuilding(POWER_PLANT, 3, {{ENERGY, 500}});
    Petition petition(42, building);

    EXPECT_EQ(petition.getId(), 42);
    EXPECT_EQ(petition.getBuilding(), building);
}

TEST(PetitionTest, BuildTickDelegatesToBuilding) {
    Petition petition(1, new StubBuilding(SOLAR_PANEL_FARM, 2, {{ENERGY, 100}}));

    const auto firstTick = petition.buildTick();
    EXPECT_TRUE(firstTick.empty());

    const auto secondTick = petition.buildTick();
    ASSERT_EQ(secondTick.size(), 1u);
    EXPECT_EQ(secondTick[0].type, ENERGY);
    EXPECT_EQ(secondTick[0].deltaValue, 100);
}

TEST(PetitionTest, DestructorDeletesBuilding) {
    bool deleted = false;

    {
        Petition petition(1, new TrackingBuilding(deleted));
        EXPECT_FALSE(deleted);
    }

    EXPECT_TRUE(deleted);
}

TEST(PetitionTest, IdsAreIndependent) {
    Petition first(1, new StubBuilding(WIND_TURBINE_FARM, 1, {}));
    Petition second(2, new StubBuilding(URBAN_GREENING, 1, {}));

    EXPECT_NE(first.getId(), second.getId());
}

TEST(PetitionManagerTest, HasCurrentPetitionOnConstruction) {
    PetitionManager petitionManager;

    EXPECT_NE(petitionManager.getCurrentPetition(), nullptr);
}

TEST(PetitionManagerTest, NoUnderConstructionPetitionsInitially) {
    PetitionManager petitionManager;

    EXPECT_TRUE(petitionManager.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, FirstPetitionIdIsOne) {
    PetitionManager petitionManager;

    EXPECT_EQ(petitionManager.getCurrentPetition()->getId(), 1);
}

TEST(PetitionManagerTest, AcceptMovesCurrentToUnderConstruction) {
    PetitionManager petitionManager;
    Petition* original = petitionManager.getCurrentPetition();

    petitionManager.acceptPetition();

    const auto& underConstruction = petitionManager.getUnderConstructionPetitions();
    ASSERT_EQ(underConstruction.size(), 1u);
    EXPECT_EQ(underConstruction[0], original);
}

TEST(PetitionManagerTest, AcceptGeneratesNewCurrentPetition) {
    PetitionManager petitionManager;
    Petition* first = petitionManager.getCurrentPetition();

    petitionManager.acceptPetition();

    EXPECT_NE(petitionManager.getCurrentPetition(), nullptr);
    EXPECT_NE(petitionManager.getCurrentPetition(), first);
}

TEST(PetitionManagerTest, AcceptIncrementsPetitionId) {
    PetitionManager petitionManager;
    const int firstId = petitionManager.getCurrentPetition()->getId();

    petitionManager.acceptPetition();

    EXPECT_EQ(petitionManager.getCurrentPetition()->getId(), firstId + 1);
}

TEST(PetitionManagerTest, MultipleAcceptsAccumulateUnderConstruction) {
    PetitionManager petitionManager;

    petitionManager.acceptPetition();
    petitionManager.acceptPetition();
    petitionManager.acceptPetition();

    EXPECT_EQ(petitionManager.getUnderConstructionPetitions().size(), 3u);
}

TEST(PetitionManagerTest, RejectDoesNotAddToUnderConstruction) {
    PetitionManager petitionManager;

    petitionManager.rejectPetition();

    EXPECT_TRUE(petitionManager.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, RejectGeneratesNewCurrentPetition) {
    PetitionManager petitionManager;
    const int firstId = petitionManager.getCurrentPetition()->getId();

    petitionManager.rejectPetition();

    EXPECT_NE(petitionManager.getCurrentPetition(), nullptr);
    EXPECT_EQ(petitionManager.getCurrentPetition()->getId(), firstId + 1);
}

TEST(PetitionManagerTest, RejectIncrementsPetitionId) {
    PetitionManager petitionManager;
    const int firstId = petitionManager.getCurrentPetition()->getId();

    petitionManager.rejectPetition();

    EXPECT_EQ(petitionManager.getCurrentPetition()->getId(), firstId + 1);
}

TEST(PetitionManagerTest, TickWithNoUnderConstructionReturnsEmpty) {
    PetitionManager petitionManager;

    const auto result = petitionManager.tick();

    EXPECT_TRUE(result.empty());
}

TEST(PetitionManagerTest, TickBeforeCompletionReturnsEmptyAndKeepsPetition) {
    PetitionManager petitionManager;
    petitionManager.restoreUnderConstruction(makePetition(10, HYDROELECTRIC_PLANT, 3, {{ENERGY, 200}}));

    const auto result = petitionManager.tick();

    EXPECT_TRUE(result.empty());
    EXPECT_EQ(petitionManager.getUnderConstructionPetitions().size(), 1u);
}

TEST(PetitionManagerTest, TickCompletesAndRemovesPetition) {
    PetitionManager petitionManager;
    petitionManager.restoreUnderConstruction(makePetition(10, POWER_PLANT, 1, {{ENERGY, 500}}));

    const auto result = petitionManager.tick();

    EXPECT_EQ(result.size(), 1u);
    EXPECT_TRUE(petitionManager.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, TickReturnsCorrectBuildingTypeOnCompletion) {
    PetitionManager petitionManager;
    petitionManager.restoreUnderConstruction(makePetition(10, WIND_TURBINE_FARM, 1, {{ENERGY, 100}}));

    const auto result = petitionManager.tick();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].type, WIND_TURBINE_FARM);
}

TEST(PetitionManagerTest, TickReturnsCorrectEffectsOnCompletion) {
    PetitionManager petitionManager;
    const std::vector<ResourceEffect> effects = {{WATER, 300}, {CO2, -50}};
    petitionManager.restoreUnderConstruction(makePetition(10, WATER_TREATMENT_PLANT, 1, effects));

    const auto result = petitionManager.tick();

    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0].effects.size(), 2u);
    EXPECT_EQ(result[0].effects[0].type, WATER);
    EXPECT_EQ(result[0].effects[0].deltaValue, 300);
    EXPECT_EQ(result[0].effects[1].type, CO2);
    EXPECT_EQ(result[0].effects[1].deltaValue, -50);
}

TEST(PetitionManagerTest, MultiTickBuildingCompletesAfterCorrectTicks) {
    PetitionManager petitionManager;
    petitionManager.restoreUnderConstruction(makePetition(10, SOLAR_PANEL_FARM, 3, {{ENERGY, 999}}));

    EXPECT_TRUE(petitionManager.tick().empty());
    EXPECT_TRUE(petitionManager.tick().empty());

    const auto result = petitionManager.tick();

    EXPECT_EQ(result.size(), 1u);
    EXPECT_TRUE(petitionManager.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, OnlyCompletedPetitionsAreRemovedEachTick) {
    PetitionManager petitionManager;
    petitionManager.restoreUnderConstruction(makePetition(10, URBAN_GREENING, 1, {{MONEY, 100}}));
    petitionManager.restoreUnderConstruction(makePetition(11, AIRPORT_EXPANSION, 3, {{MONEY, 9999}}));

    const auto result = petitionManager.tick();

    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(petitionManager.getUnderConstructionPetitions().size(), 1u);
    EXPECT_EQ(petitionManager.getUnderConstructionPetitions()[0]->getId(), 11);
}

TEST(PetitionManagerTest, MultipleCompletionsInOneTick) {
    PetitionManager petitionManager;
    petitionManager.restoreUnderConstruction(makePetition(20, ROAD_IMPROVEMENT, 1, {{MONEY, 50}}));
    petitionManager.restoreUnderConstruction(makePetition(21, INDUSTRIAL_ZONE, 1, {{ENERGY, 75}}));

    const auto result = petitionManager.tick();

    EXPECT_EQ(result.size(), 2u);
    EXPECT_TRUE(petitionManager.getUnderConstructionPetitions().empty());
}

TEST(PetitionManagerTest, GeneratedPetitionHasValidBuildingType) {
    PetitionManager petitionManager;
    static const std::array<BuildingType, 12> validTypes = {
        POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM,
        SOLAR_PANEL_ROOFTOPS, PUBLIC_TRANSPORT_UPGRADE, WIND_TURBINE_FARM,
        HYDROELECTRIC_PLANT, URBAN_GREENING, WATER_SAVING_INFRASTRUCTURE,
        INDUSTRIAL_ZONE, AIRPORT_EXPANSION, ROAD_IMPROVEMENT
    };

    for (int i = 0; i < 50; ++i) {
        Petition* petition = petitionManager.generatePetition();
        const BuildingType type = petition->getBuilding()->getType();
        const bool found = std::find(validTypes.begin(), validTypes.end(), type) != validTypes.end();
        EXPECT_TRUE(found) << "Unexpected building type: " << type;
        delete petition;
    }
}

TEST(PetitionManagerTest, GeneratedPetitionsHaveMonotonicallyIncreasingIds) {
    PetitionManager petitionManager;
    int lastId = petitionManager.getCurrentPetition()->getId();

    for (int i = 0; i < 10; ++i) {
        Petition* petition = petitionManager.generatePetition();
        EXPECT_GT(petition->getId(), lastId);
        lastId = petition->getId();
        delete petition;
    }
}

TEST(PetitionManagerTest, AcceptThenTickWithRealBuildingDoesNotCrash) {
    PetitionManager petitionManager;

    petitionManager.acceptPetition();

    EXPECT_NO_THROW({
        const auto result = petitionManager.tick();
        (void)result;
    });
}

TEST(PetitionManagerTest, RejectThenAcceptThenTickWithRealBuildingDoesNotCrash) {
    PetitionManager petitionManager;

    petitionManager.rejectPetition();
    petitionManager.acceptPetition();

    EXPECT_NO_THROW({
        const auto result = petitionManager.tick();
        (void)result;
    });
}

TEST(PetitionManagerTest, RestoreCurrentPetitionReplacesAutoGeneratedPetition) {
    PetitionManager petitionManager;

    petitionManager.restoreCurrentPetition(makePetition(77, SOLAR_PANEL_ROOFTOPS, 2, {{ENERGY, 75}}));

    ASSERT_NE(petitionManager.getCurrentPetition(), nullptr);
    EXPECT_EQ(petitionManager.getCurrentPetition()->getId(), 77);
    EXPECT_EQ(petitionManager.getCurrentPetition()->getBuilding()->getType(), SOLAR_PANEL_ROOFTOPS);
}

TEST(PetitionManagerTest, RestoreHelpersAdvanceNextGeneratedId) {
    PetitionManager petitionManager;

    petitionManager.restoreCurrentPetition(makePetition(100, SOLAR_PANEL_ROOFTOPS, 2, {{ENERGY, 75}}));
    Petition* generated = petitionManager.generatePetition();

    EXPECT_GT(generated->getId(), 100);
    delete generated;
}
