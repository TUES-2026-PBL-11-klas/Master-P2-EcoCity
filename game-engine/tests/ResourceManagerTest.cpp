#include <gtest/gtest.h>

#include "../domain/Resource.hpp"
#include "../domain/ResourceEffect.hpp"
#include "../services/ResourceManager.hpp"

#include <vector>

TEST(ResourceTest, InitialValues) {
    Resource resource(WATER, 500, 10);

    EXPECT_EQ(resource.getCurrentValue(), 500);
    EXPECT_EQ(resource.getDeltaValue(), 10);
    EXPECT_EQ(resource.getType(), WATER);
}

TEST(ResourceTest, ChangeCurrentValueByDelta) {
    Resource resource(ENERGY, 100, 0);

    resource.changeCurrentValue(50);
    EXPECT_EQ(resource.getCurrentValue(), 150);

    resource.changeCurrentValue(-200);
    EXPECT_EQ(resource.getCurrentValue(), 0);
}

TEST(ResourceTest, ChangeCurrentValueByTick) {
    Resource resource(MONEY, 1000, 200);

    resource.changeCurrentValue();
    EXPECT_EQ(resource.getCurrentValue(), 1200);
}

TEST(ResourceTest, ClampAtMaxValue) {
    Resource resource(CO2, MAX_RESOURCE_VALUE - 5, 0);

    resource.changeCurrentValue(100);
    EXPECT_EQ(resource.getCurrentValue(), MAX_RESOURCE_VALUE);
}

TEST(ResourceTest, ClampAtZero) {
    Resource resource(POPULATION, 10, -50);

    resource.changeCurrentValue();
    EXPECT_EQ(resource.getCurrentValue(), 0);
}

TEST(ResourceTest, ChangeDeltaPerTickAccumulates) {
    Resource resource(WATER, 0, 10);

    resource.changeDeltaPerTick(5);
    EXPECT_EQ(resource.getDeltaValue(), 15);

    resource.changeDeltaPerTick(-20);
    EXPECT_EQ(resource.getDeltaValue(), -5);
}

TEST(ResourceManagerTest, DefaultInitialValues) {
    ResourceManager resourceManager;

    EXPECT_EQ(resourceManager.getResourceValue(WATER), 1'000'000LL);
    EXPECT_EQ(resourceManager.getResourceValue(ENERGY), 5'000'000LL);
    EXPECT_EQ(resourceManager.getResourceValue(MONEY), 250'000LL);
    EXPECT_EQ(resourceManager.getResourceValue(POPULATION), 1'000'000LL);
    EXPECT_EQ(resourceManager.getResourceValue(CO2), 1'000'000LL);
}

TEST(ResourceManagerTest, GetResourcesReturnsAllFive) {
    ResourceManager resourceManager;

    EXPECT_EQ(resourceManager.getResources().size(), 5u);
}

TEST(ResourceManagerTest, TickWithZeroDeltaChangesNothing) {
    ResourceManager resourceManager;
    const LLint before = resourceManager.getResourceValue(WATER);

    resourceManager.tick();

    EXPECT_EQ(resourceManager.getResourceValue(WATER), before);
}

TEST(ResourceManagerTest, TickAppliesDelta) {
    ResourceManager resourceManager;
    const LLint before = resourceManager.getResourceValue(WATER);

    resourceManager.applyEffect({{WATER, 500}});
    resourceManager.tick();

    EXPECT_EQ(resourceManager.getResourceValue(WATER), before + 500);
}

TEST(ResourceManagerTest, MultipleTicksAccumulateDelta) {
    ResourceManager resourceManager;
    const LLint before = resourceManager.getResourceValue(MONEY);

    resourceManager.applyEffect({{MONEY, 1000}});
    resourceManager.tick();
    resourceManager.tick();
    resourceManager.tick();

    EXPECT_EQ(resourceManager.getResourceValue(MONEY), before + 3000);
}

TEST(ResourceManagerTest, ApplyEffectChangesDelta) {
    ResourceManager resourceManager;

    resourceManager.applyEffect({{ENERGY, 200}});

    EXPECT_EQ(resourceManager.getDeltaForResourceType(ENERGY), 200);
}

TEST(ResourceManagerTest, ApplyMultipleEffects) {
    ResourceManager resourceManager;
    const std::vector<ResourceEffect> effects = {
        {WATER, 100},
        {ENERGY, -50},
        {CO2, 300}
    };

    resourceManager.applyEffect(effects);

    EXPECT_EQ(resourceManager.getDeltaForResourceType(WATER), 100);
    EXPECT_EQ(resourceManager.getDeltaForResourceType(ENERGY), -50);
    EXPECT_EQ(resourceManager.getDeltaForResourceType(CO2), 300);
}

TEST(ResourceManagerTest, ApplyEffectSkipsUnspecified) {
    ResourceManager resourceManager;
    const LLint before = resourceManager.getResourceValue(WATER);

    resourceManager.applyEffect({{RESOURCE_UNSPECIFIED, 9999}});
    resourceManager.tick();

    EXPECT_EQ(resourceManager.getResourceValue(WATER), before);
}

TEST(ResourceManagerTest, ApplyEffectAccumulates) {
    ResourceManager resourceManager;

    resourceManager.applyEffect({{MONEY, 100}});
    resourceManager.applyEffect({{MONEY, 200}});

    EXPECT_EQ(resourceManager.getDeltaForResourceType(MONEY), 300);
}

TEST(ResourceManagerTest, CanAffordWhenSufficientFunds) {
    ResourceManager resourceManager;

    EXPECT_TRUE(resourceManager.canAfford(100'000LL));
    EXPECT_TRUE(resourceManager.canAfford(250'000LL));
}

TEST(ResourceManagerTest, CannotAffordWhenInsufficientFunds) {
    ResourceManager resourceManager;

    EXPECT_FALSE(resourceManager.canAfford(250'001LL));
    EXPECT_FALSE(resourceManager.canAfford(1'000'000LL));
}

TEST(ResourceManagerTest, CanAffordZero) {
    ResourceManager resourceManager;

    EXPECT_TRUE(resourceManager.canAfford(0));
}

TEST(ResourceManagerTest, ChangeResourceValueIncrease) {
    ResourceManager resourceManager;
    const LLint before = resourceManager.getResourceValue(CO2);

    resourceManager.changeResourceValue(CO2, 5000);

    EXPECT_EQ(resourceManager.getResourceValue(CO2), before + 5000);
}

TEST(ResourceManagerTest, ChangeResourceValueDecrease) {
    ResourceManager resourceManager;
    const LLint before = resourceManager.getResourceValue(POPULATION);

    resourceManager.changeResourceValue(POPULATION, -500'000LL);

    EXPECT_EQ(resourceManager.getResourceValue(POPULATION), before - 500'000LL);
}

TEST(ResourceManagerTest, ChangeResourceValueClampsAtZero) {
    ResourceManager resourceManager;

    resourceManager.changeResourceValue(WATER, -999'999'999LL);

    EXPECT_EQ(resourceManager.getResourceValue(WATER), 0);
}

TEST(ResourceManagerTest, ChangeResourceValueIgnoresUnspecified) {
    ResourceManager resourceManager;
    const LLint before = resourceManager.getResourceValue(WATER);

    resourceManager.changeResourceValue(RESOURCE_UNSPECIFIED, 9999);

    EXPECT_EQ(resourceManager.getResourceValue(WATER), before);
}

TEST(ResourceManagerTest, IteratorCoversAllResources) {
    ResourceManager resourceManager;
    int count = 0;

    for (const Resource& resource : resourceManager) {
        (void)resource;
        ++count;
    }

    EXPECT_EQ(count, 5);
}
