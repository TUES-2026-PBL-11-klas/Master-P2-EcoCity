#include <gtest/gtest.h>

#include "../domain/City.hpp"
#include "../domain/Petition.hpp"
#include "../network/ISocketServer.hpp"
#include "../persistence/IGameRepository.hpp"
#include "../services/GameService.hpp"

#include <optional>
#include <string>
#include <unordered_map>
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

class FakeSocketServer : public ISocketServer {
    public:
        std::optional<game_api::v1::UIAction> nextAction;
        std::vector<game_api::v1::GameState> sentStates;
        bool stopped = false;

        std::optional<game_api::v1::UIAction> pollAction() override
        {
            auto action = nextAction;
            nextAction.reset();
            return action;
        }

        void sendGameState(const game_api::v1::GameState& state) override
        {
            sentStates.push_back(state);
        }

        void stop() override
        {
            stopped = true;
        }
};

class FakeGameRepository : public IGameRepository {
    public:
        int saveCalls = 0;
        std::string lastGameId;

        void saveGame(
            const std::string& gameId,
            const ResourceManager&,
            const PetitionManager&,
            const City&
        ) override
        {
            ++saveCalls;
            lastGameId = gameId;
        }
};

Petition* makePetition(int id, BuildingType type, int ticksToComplete, std::vector<ResourceEffect> effects)
{
    return new Petition(id, new StubBuilding(type, ticksToComplete, std::move(effects)));
}

}

class GameServiceTest : public ::testing::Test {
    protected:
        ResourceManager resourceManager;
        PetitionManager petitionManager;
        City city;
        FakeSocketServer socketServer;
        FakeGameRepository gameRepository;
        GameService gameService{&resourceManager, &petitionManager, &city, &socketServer, &gameRepository, "test-game"};
};

TEST(CityTest, AllBuildingCountsInitiallyZero) {
    City city;

    for (auto type : {POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM,
                      SOLAR_PANEL_ROOFTOPS, PUBLIC_TRANSPORT_UPGRADE, WIND_TURBINE_FARM,
                      HYDROELECTRIC_PLANT, URBAN_GREENING, WATER_SAVING_INFRASTRUCTURE,
                      INDUSTRIAL_ZONE, AIRPORT_EXPANSION, ROAD_IMPROVEMENT}) {
        EXPECT_EQ(city.getBuildingCount(type), 0) << "Expected 0 for type " << type;
    }
}

TEST(CityTest, AddBuildingIncrementsByOne) {
    City city;

    city.addBuilding(POWER_PLANT);

    EXPECT_EQ(city.getBuildingCount(POWER_PLANT), 1);
}

TEST(CityTest, AddSameBuildingMultipleTimes) {
    City city;

    city.addBuilding(SOLAR_PANEL_FARM);
    city.addBuilding(SOLAR_PANEL_FARM);
    city.addBuilding(SOLAR_PANEL_FARM);

    EXPECT_EQ(city.getBuildingCount(SOLAR_PANEL_FARM), 3);
}

TEST(CityTest, AddDifferentBuildingsAreIndependent) {
    City city;

    city.addBuilding(WIND_TURBINE_FARM);
    city.addBuilding(HYDROELECTRIC_PLANT);
    city.addBuilding(HYDROELECTRIC_PLANT);

    EXPECT_EQ(city.getBuildingCount(WIND_TURBINE_FARM), 1);
    EXPECT_EQ(city.getBuildingCount(HYDROELECTRIC_PLANT), 2);
    EXPECT_EQ(city.getBuildingCount(ROAD_IMPROVEMENT), 0);
}

TEST(CityTest, GetBuildingsReturnsAllEntries) {
    City city;

    EXPECT_EQ(city.getBuildings().size(), 13u);
}

TEST(CityTest, GetBuildingsReflectsAddBuilding) {
    City city;

    city.addBuilding(INDUSTRIAL_ZONE);

    EXPECT_EQ(city.getBuildings().at(INDUSTRIAL_ZONE), 1);
}

TEST_F(GameServiceTest, TickSendsGameStateToSocketServer) {
    EXPECT_FALSE(gameService.tick());

    ASSERT_EQ(socketServer.sentStates.size(), 1u);
    EXPECT_TRUE(socketServer.sentStates.back().has_current_petition());
}

TEST_F(GameServiceTest, CompletedConstructionAddsBuildingToCity) {
    petitionManager.restoreUnderConstruction(makePetition(55, POWER_PLANT, 1, {{ENERGY, 500}}));

    gameService.tick();

    EXPECT_EQ(city.getBuildingCount(POWER_PLANT), 1);
}

TEST_F(GameServiceTest, CompletedConstructionAppliesEffectsAsDeltas) {
    const LLint waterBefore = resourceManager.getResourceValue(WATER);
    const LLint deltaBefore = resourceManager.getDeltaForResourceType(WATER);
    petitionManager.restoreUnderConstruction(makePetition(55, WATER_TREATMENT_PLANT, 1, {{WATER, 200}}));

    gameService.tick();

    EXPECT_EQ(resourceManager.getResourceValue(WATER), waterBefore + deltaBefore + 200);
}

TEST_F(GameServiceTest, MultipleCompletionsInOneTick) {
    const LLint energyBefore = resourceManager.getResourceValue(ENERGY);
    const LLint deltaBefore = resourceManager.getDeltaForResourceType(ENERGY);
    petitionManager.restoreUnderConstruction(makePetition(10, SOLAR_PANEL_FARM, 1, {{ENERGY, 100}}));
    petitionManager.restoreUnderConstruction(makePetition(11, WIND_TURBINE_FARM, 1, {{ENERGY, 150}}));

    gameService.tick();

    EXPECT_EQ(city.getBuildingCount(SOLAR_PANEL_FARM), 1);
    EXPECT_EQ(city.getBuildingCount(WIND_TURBINE_FARM), 1);
    EXPECT_EQ(resourceManager.getResourceValue(ENERGY), energyBefore + deltaBefore + 250);
}

TEST_F(GameServiceTest, NoCompletionsLeaveCityUnchanged) {
    gameService.tick();

    for (auto type : {POWER_PLANT, WATER_TREATMENT_PLANT, SOLAR_PANEL_FARM}) {
        EXPECT_EQ(city.getBuildingCount(type), 0);
    }
}

TEST_F(GameServiceTest, ResourceManagerTickedEachGameTick) {
    resourceManager.applyEffect({{MONEY, 1000}});
    const LLint before = resourceManager.getResourceValue(MONEY);
    const LLint delta = resourceManager.getDeltaForResourceType(MONEY);

    gameService.tick();

    EXPECT_EQ(resourceManager.getResourceValue(MONEY), before + delta);
}

TEST_F(GameServiceTest, NoGameOverWithHealthyResources) {
    EXPECT_FALSE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenCO2ReachesMax) {
    resourceManager.changeResourceValue(CO2, MAX_CO2 - resourceManager.getResourceValue(CO2));

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenCO2ExceedsMax) {
    resourceManager.changeResourceValue(CO2, (MAX_CO2 + 1) - resourceManager.getResourceValue(CO2));

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, NoGameOverWhenCO2JustBelowMax) {
    resourceManager.setDeltaForResourceType(CO2, 0);
    resourceManager.changeResourceValue(CO2, (MAX_CO2 - 1) - resourceManager.getResourceValue(CO2));

    EXPECT_FALSE(gameService.tick());
}

TEST_F(GameServiceTest, CO2DepletedIsNotGameOver) {
    resourceManager.changeResourceValue(CO2, -resourceManager.getResourceValue(CO2));

    EXPECT_FALSE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenWaterDepleted) {
    resourceManager.changeResourceValue(WATER, -resourceManager.getResourceValue(WATER));

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenEnergyDepleted) {
    resourceManager.changeResourceValue(ENERGY, -resourceManager.getResourceValue(ENERGY));

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenMoneyDepleted) {
    resourceManager.setDeltaForResourceType(MONEY, 0);
    resourceManager.changeResourceValue(MONEY, -resourceManager.getResourceValue(MONEY));

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenPopulationDepleted) {
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.changeResourceValue(POPULATION, -resourceManager.getResourceValue(POPULATION));

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, NoGameOverWhenResourcesAtOne) {
    resourceManager.setDeltaForResourceType(WATER, 0);
    resourceManager.setDeltaForResourceType(ENERGY, 0);
    resourceManager.setDeltaForResourceType(MONEY, 0);
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(CO2, 0);
    resourceManager.changeResourceValue(WATER, 1 - resourceManager.getResourceValue(WATER));
    resourceManager.changeResourceValue(ENERGY, 1 - resourceManager.getResourceValue(ENERGY));
    resourceManager.changeResourceValue(MONEY, 1 - resourceManager.getResourceValue(MONEY));
    resourceManager.changeResourceValue(POPULATION, 1 - resourceManager.getResourceValue(POPULATION));
    resourceManager.changeResourceValue(CO2, 1 - resourceManager.getResourceValue(CO2));

    EXPECT_FALSE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenResourceTicksDownToZero) {
    resourceManager.changeResourceValue(MONEY, 10 - resourceManager.getResourceValue(MONEY));
    resourceManager.setDeltaForResourceType(MONEY, -10);

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, NoGameOverBeforeResourceReachesZero) {
    resourceManager.changeResourceValue(MONEY, 100 - resourceManager.getResourceValue(MONEY));
    resourceManager.applyEffect({{MONEY, -10}});

    EXPECT_FALSE(gameService.tick());
}

TEST_F(GameServiceTest, GameOverWhenBothConditionsMet) {
    resourceManager.changeResourceValue(CO2, MAX_CO2 - resourceManager.getResourceValue(CO2));
    resourceManager.changeResourceValue(WATER, -resourceManager.getResourceValue(WATER));

    EXPECT_TRUE(gameService.tick());
}

TEST_F(GameServiceTest, AcceptedPetitionMovesIntoConstructionQueue) {
    Petition* current = petitionManager.getCurrentPetition();
    game_api::v1::UIAction action;
    action.mutable_petition_response()->set_responded(true);
    action.mutable_petition_response()->set_accepted(true);
    socketServer.nextAction = action;

    gameService.tick();

    ASSERT_EQ(petitionManager.getUnderConstructionPetitions().size(), 1u);
    EXPECT_EQ(petitionManager.getUnderConstructionPetitions()[0], current);
}

TEST_F(GameServiceTest, RejectedPetitionDoesNotEnterConstructionQueue) {
    Petition* current = petitionManager.getCurrentPetition();
    game_api::v1::UIAction action;
    action.mutable_petition_response()->set_responded(true);
    action.mutable_petition_response()->set_accepted(false);
    socketServer.nextAction = action;

    gameService.tick();

    EXPECT_TRUE(petitionManager.getUnderConstructionPetitions().empty());
    EXPECT_NE(petitionManager.getCurrentPetition(), current);
}

TEST_F(GameServiceTest, SaveActionCallsRepository) {
    game_api::v1::UIAction action;
    action.set_save_game(true);
    socketServer.nextAction = action;

    gameService.tick();

    EXPECT_EQ(gameRepository.saveCalls, 1);
    EXPECT_EQ(gameRepository.lastGameId, "test-game");
}

TEST_F(GameServiceTest, SentGameStateIncludesCompletedBuildingCounts) {
    petitionManager.restoreUnderConstruction(makePetition(90, ROAD_IMPROVEMENT, 1, {{MONEY, 50}}));

    gameService.tick();

    ASSERT_FALSE(socketServer.sentStates.empty());
    const auto& state = socketServer.sentStates.back();
    const auto counts = state.building_counts();
    const auto iterator = counts.find(static_cast<int>(ROAD_IMPROVEMENT));
    ASSERT_NE(iterator, counts.end());
    EXPECT_EQ(iterator->second, 1);
}

TEST_F(GameServiceTest, SentGameStateIncludesResourceValues) {
    gameService.tick();

    ASSERT_FALSE(socketServer.sentStates.empty());
    const auto& state = socketServer.sentStates.back();
    const auto& resources = state.resources();
    EXPECT_TRUE(resources.count(WATER));
    EXPECT_TRUE(resources.count(ENERGY));
    EXPECT_TRUE(resources.count(MONEY));
    EXPECT_TRUE(resources.count(POPULATION));
    EXPECT_TRUE(resources.count(CO2));
}

// ── handlePopulationScaling ──────────────────────────────────────────────────

TEST_F(GameServiceTest, PopulationBelowGoalDoesNotScaleDeltas) {
    // Default population (1 000 000) is below nextPopulationGoal (1 200 000)
    resourceManager.setDeltaForResourceType(WATER, -10000);
    const LLint waterDeltaBefore = resourceManager.getDeltaForResourceType(WATER);

    gameService.handlePopulationScaling();

    EXPECT_EQ(resourceManager.getDeltaForResourceType(WATER), waterDeltaBefore);
}

TEST_F(GameServiceTest, PopulationAtGoalTriggersScaling) {
    // Push population up to exactly the goal
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(WATER, -10000);
    const LLint expected = static_cast<LLint>(-10000 * demandIncrease); // -11000

    gameService.handlePopulationScaling();

    EXPECT_EQ(resourceManager.getDeltaForResourceType(WATER), expected);
}

TEST_F(GameServiceTest, PopulationScalingWaterNegativeDeltaIncreasesConsumption) {
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(WATER, -10000);

    gameService.handlePopulationScaling();

    // Negative delta grows more negative: -10000 * 1.10 = -11000
    EXPECT_EQ(resourceManager.getDeltaForResourceType(WATER), -11000LL);
}

TEST_F(GameServiceTest, PopulationScalingWaterPositiveDeltaDecreases) {
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(WATER, 5000);

    gameService.handlePopulationScaling();

    // Positive delta shrinks: 5000 * (2 - 1.10) = 5000 * 0.90 = 4500
    EXPECT_EQ(resourceManager.getDeltaForResourceType(WATER), static_cast<LLint>(5000 * (2 - demandIncrease)));
}

TEST_F(GameServiceTest, PopulationScalingEnergyNegativeDeltaIncreasesConsumption) {
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(ENERGY, -8000);

    gameService.handlePopulationScaling();

    EXPECT_EQ(resourceManager.getDeltaForResourceType(ENERGY), static_cast<LLint>(-8000 * demandIncrease));
}

TEST_F(GameServiceTest, PopulationScalingCO2PositiveDeltaIncreases) {
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(CO2, 10000);

    gameService.handlePopulationScaling();

    EXPECT_EQ(resourceManager.getDeltaForResourceType(CO2), static_cast<LLint>(10000 * demandIncrease));
}

TEST_F(GameServiceTest, PopulationScalingCO2NegativeDeltaChanges) {
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(CO2, -5000);

    gameService.handlePopulationScaling();

    // newDelta = currentDelta - currentDelta * (2 - demandIncrease)
    // = -5000 - (-5000 * 0.90) = -5000 + 4500 = -500
    const LLint currentDelta = -5000;
    const LLint expected = currentDelta - static_cast<LLint>(currentDelta * (2 - demandIncrease));
    EXPECT_EQ(resourceManager.getDeltaForResourceType(CO2), expected);
}

TEST_F(GameServiceTest, PopulationScalingMoneyDeltaIncreases) {
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(MONEY, 10000);

    gameService.handlePopulationScaling();

    EXPECT_EQ(resourceManager.getDeltaForResourceType(MONEY), static_cast<LLint>(10000 * demandIncrease));
}

TEST_F(GameServiceTest, PopulationScalingDoesNotChangePopulationDelta) {
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    const LLint popDeltaBefore = resourceManager.getDeltaForResourceType(POPULATION);
    resourceManager.setDeltaForResourceType(POPULATION, 0);

    gameService.handlePopulationScaling();

    // POPULATION has no branch in handlePopulationScaling — its delta is unchanged
    EXPECT_EQ(resourceManager.getDeltaForResourceType(POPULATION), 0);
    (void)popDeltaBefore;
}

TEST_F(GameServiceTest, PopulationScalingNextGoalAdvancesByScalingFactor) {
    // Trigger scaling once — then confirm the next tick at same population does NOT scale again
    resourceManager.changeResourceValue(POPULATION, 1200000 - resourceManager.getResourceValue(POPULATION));
    resourceManager.setDeltaForResourceType(POPULATION, 0);
    resourceManager.setDeltaForResourceType(WATER, -10000);

    gameService.handlePopulationScaling(); // triggers, goal becomes 1 440 000
    const LLint deltaAfterFirst = resourceManager.getDeltaForResourceType(WATER);

    gameService.handlePopulationScaling(); // population still 1 200 000 < 1 440 000 → no-op

    EXPECT_EQ(resourceManager.getDeltaForResourceType(WATER), deltaAfterFirst);
}

// ── Building getters, setters and guards ─────────────────────────────────────

namespace {
class ConcreteBuilding : public Building {
    public:
        ConcreteBuilding(BuildingType type, LLint cost, int ticks, std::vector<ResourceEffect> fx)
            : Building(type, cost, ticks) { effects = std::move(fx); }
    private:
        std::vector<ResourceEffect> Effects() const override { return effects; }
};
}

TEST(BuildingTest, GettersReturnConstructorValues) {
    ConcreteBuilding b(POWER_PLANT, 5000, 3, {{ENERGY, 500}});

    EXPECT_EQ(b.getType(), POWER_PLANT);
    EXPECT_EQ(b.getBuildCost(), 5000LL);
    EXPECT_EQ(b.getTicksToComplete(), 3);
    ASSERT_EQ(b.getEffects().size(), 1u);
    EXPECT_EQ(b.getEffects()[0].type, ENERGY);
    EXPECT_EQ(b.getEffects()[0].deltaValue, 500);
}

TEST(BuildingTest, SetTicksToCompleteUpdatesValue) {
    ConcreteBuilding b(SOLAR_PANEL_FARM, 0, 1, {});

    b.setTicksToComplete(7);

    EXPECT_EQ(b.getTicksToComplete(), 7);
}

TEST(BuildingTest, SetTicksToCompleteNegativeClampsToZero) {
    ConcreteBuilding b(SOLAR_PANEL_FARM, 0, 5, {});

    b.setTicksToComplete(-3);

    EXPECT_EQ(b.getTicksToComplete(), 0);
}

TEST(BuildingTest, SetBuildCostUpdatesValue) {
    ConcreteBuilding b(WIND_TURBINE_FARM, 1000, 1, {});

    b.setBuildCost(9999);

    EXPECT_EQ(b.getBuildCost(), 9999LL);
}

TEST(BuildingTest, SetBuildCostNegativeClampsToZero) {
    ConcreteBuilding b(WIND_TURBINE_FARM, 1000, 1, {});

    b.setBuildCost(-500);

    EXPECT_EQ(b.getBuildCost(), 0LL);
}

TEST(BuildingTest, BuildTickAlreadyCompletedReturnsEmpty) {
    ConcreteBuilding b(URBAN_GREENING, 0, 1, {{MONEY, 100}});
    b.buildTick(); // completes the building (ticks = 0 now)

    const auto result = b.buildTick(); // called again after completion

    EXPECT_TRUE(result.empty());
}
