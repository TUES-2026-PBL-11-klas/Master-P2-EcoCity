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
