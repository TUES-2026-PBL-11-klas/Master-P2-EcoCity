#include <gtest/gtest.h>
#include <array>
#include <vector>

// ---------------------------------------------------------------
// Inline the headers/sources so the test file is self-contained.
// In a real project you would just #include your actual headers.
// ---------------------------------------------------------------

// ---- ResourceType.hpp ----
enum ResourceType {
    RESOURCE_UNSPECIFIED,
    WATER,
    ENERGY,
    MONEY,
    POPULATION,
    CO2
};

// ---- Resource.hpp / Resource.cpp ----
typedef long long int LLint;
constexpr LLint MAX_RESOURCE_VALUE = 1'000'000'000LL;

class Resource {
    ResourceType type;
    LLint currentValue;
    LLint deltaPerTick;
public:
    Resource(ResourceType t, LLint amount, LLint delta)
        : type(t), currentValue(amount), deltaPerTick(delta) {}

    ResourceType getType()     const { return type; }
    LLint getCurrentValue()    const { return currentValue; }
    LLint getDeltaValue()      const { return deltaPerTick; }

    void changeDeltaPerTick(LLint delta) { deltaPerTick += delta; }

    void changeCurrentValue(LLint delta) {
        currentValue += delta;
        if (currentValue < 0)                   currentValue = 0;
        if (currentValue > MAX_RESOURCE_VALUE)  currentValue = MAX_RESOURCE_VALUE;
    }

    void changeCurrentValue() {
        currentValue += deltaPerTick;
        if (currentValue < 0)                   currentValue = 0;
        if (currentValue > MAX_RESOURCE_VALUE)  currentValue = MAX_RESOURCE_VALUE;
    }
};

// ---- ResourceEffect.hpp ----
struct ResourceEffect {
    ResourceType type;
    LLint        deltaValue;
};

// ---- ResourceManager.hpp / ResourceManager.cpp ----
#include <array>
#include <vector>

class ResourceManager {
    std::array<Resource, 5> resources;

    int getIndexForResourceType(ResourceType type) const {
        return static_cast<int>(type) - 1;
    }

public:
    using ResourceArray = std::array<Resource, 5>;

    std::array<Resource, 5>::iterator       begin()       { return resources.begin(); }
    std::array<Resource, 5>::iterator       end()         { return resources.end();   }
    std::array<Resource, 5>::const_iterator begin() const { return resources.begin(); }
    std::array<Resource, 5>::const_iterator end()   const { return resources.end();   }

    ResourceManager()
        : resources{
            Resource(WATER,      1'000'000LL, 0),
            Resource(ENERGY,     5'000'000LL, 0),
            Resource(MONEY,        250'000LL, 0),
            Resource(POPULATION, 1'000'000LL, 0),
            Resource(CO2,        1'000'000LL, 0)
          }
    {
        static_assert(static_cast<int>(WATER)      == 1, "WATER index");
        static_assert(static_cast<int>(ENERGY)     == 2, "ENERGY index");
        static_assert(static_cast<int>(MONEY)      == 3, "MONEY index");
        static_assert(static_cast<int>(POPULATION) == 4, "POPULATION index");
        static_assert(static_cast<int>(CO2)        == 5, "CO2 index");
    }

    void tick() {
        for (Resource& r : resources) r.changeCurrentValue();
    }

    void applyEffect(const std::vector<ResourceEffect>& effects) {
        for (const ResourceEffect& e : effects) {
            if (e.type == RESOURCE_UNSPECIFIED) continue;
            resources[getIndexForResourceType(e.type)].changeDeltaPerTick(e.deltaValue);
        }
    }

    int getResourceValue(ResourceType type) const {
        return resources[getIndexForResourceType(type)].getCurrentValue();
    }

    bool canAfford(LLint amount) const {
        return getResourceValue(MONEY) >= amount;
    }

    void changeResourceValue(ResourceType type, LLint delta) {
        if (type == RESOURCE_UNSPECIFIED) return;
        resources[getIndexForResourceType(type)].changeCurrentValue(delta);
    }

    const ResourceArray& getResources() const { return resources; }
};

// ================================================================
// Tests
// ================================================================

// ----------------------------------------------------------------
// Resource – unit tests
// ----------------------------------------------------------------

TEST(ResourceTest, InitialValues) {
    Resource r(WATER, 500, 10);
    EXPECT_EQ(r.getCurrentValue(), 500);
    EXPECT_EQ(r.getDeltaValue(),    10);
    EXPECT_EQ(r.getType(),         WATER);
}

TEST(ResourceTest, ChangeCurrentValueByDelta) {
    Resource r(ENERGY, 100, 0);
    r.changeCurrentValue(50);
    EXPECT_EQ(r.getCurrentValue(), 150);

    r.changeCurrentValue(-200);
    EXPECT_EQ(r.getCurrentValue(), 0); // clamped at 0
}

TEST(ResourceTest, ChangeCurrentValueByTick) {
    Resource r(MONEY, 1000, 200);
    r.changeCurrentValue();           // tick
    EXPECT_EQ(r.getCurrentValue(), 1200);
}

TEST(ResourceTest, ClampAtMaxValue) {
    Resource r(CO2, MAX_RESOURCE_VALUE - 5, 0);
    r.changeCurrentValue(100);        // would exceed max
    EXPECT_EQ(r.getCurrentValue(), MAX_RESOURCE_VALUE);
}

TEST(ResourceTest, ClampAtZero) {
    Resource r(POPULATION, 10, -50);
    r.changeCurrentValue();           // tick drives it negative
    EXPECT_EQ(r.getCurrentValue(), 0);
}

TEST(ResourceTest, ChangeDeltaPerTickAccumulates) {
    Resource r(WATER, 0, 10);
    r.changeDeltaPerTick(5);
    EXPECT_EQ(r.getDeltaValue(), 15);
    r.changeDeltaPerTick(-20);
    EXPECT_EQ(r.getDeltaValue(), -5);
}

// ----------------------------------------------------------------
// ResourceManager – construction
// ----------------------------------------------------------------

TEST(ResourceManagerTest, DefaultInitialValues) {
    ResourceManager rm;
    EXPECT_EQ(rm.getResourceValue(WATER),      1'000'000LL);
    EXPECT_EQ(rm.getResourceValue(ENERGY),     5'000'000LL);
    EXPECT_EQ(rm.getResourceValue(MONEY),        250'000LL);
    EXPECT_EQ(rm.getResourceValue(POPULATION), 1'000'000LL);
    EXPECT_EQ(rm.getResourceValue(CO2),        1'000'000LL);
}

TEST(ResourceManagerTest, GetResourcesReturnsAllFive) {
    ResourceManager rm;
    EXPECT_EQ(rm.getResources().size(), 5u);
}

// ----------------------------------------------------------------
// ResourceManager – tick
// ----------------------------------------------------------------

TEST(ResourceManagerTest, TickWithZeroDeltaChangesNothing) {
    ResourceManager rm;
    LLint before = rm.getResourceValue(WATER);
    rm.tick();
    EXPECT_EQ(rm.getResourceValue(WATER), before);
}

TEST(ResourceManagerTest, TickAppliesDelta) {
    ResourceManager rm;
    rm.applyEffect({{WATER, 500}});   // set delta = 500
    LLint before = rm.getResourceValue(WATER);
    rm.tick();
    EXPECT_EQ(rm.getResourceValue(WATER), before + 500);
}

TEST(ResourceManagerTest, MultipleTicks) {
    ResourceManager rm;
    rm.applyEffect({{MONEY, 1000}});
    LLint before = rm.getResourceValue(MONEY);
    rm.tick();
    rm.tick();
    rm.tick();
    EXPECT_EQ(rm.getResourceValue(MONEY), before + 3000);
}

// ----------------------------------------------------------------
// ResourceManager – applyEffect
// ----------------------------------------------------------------

TEST(ResourceManagerTest, ApplyEffectChangesDelta) {
    ResourceManager rm;
    rm.applyEffect({{ENERGY, 200}});
    // After one tick the value should increase by 200
    LLint before = rm.getResourceValue(ENERGY);
    rm.tick();
    EXPECT_EQ(rm.getResourceValue(ENERGY), before + 200);
}

TEST(ResourceManagerTest, ApplyMultipleEffects) {
    ResourceManager rm;
    std::vector<ResourceEffect> effects = {
        {WATER,  100},
        {ENERGY, -50},
        {CO2,    300}
    };
    rm.applyEffect(effects);
    LLint wBefore = rm.getResourceValue(WATER);
    LLint eBefore = rm.getResourceValue(ENERGY);
    LLint cBefore = rm.getResourceValue(CO2);
    rm.tick();
    EXPECT_EQ(rm.getResourceValue(WATER),  wBefore + 100);
    EXPECT_EQ(rm.getResourceValue(ENERGY), eBefore - 50);
    EXPECT_EQ(rm.getResourceValue(CO2),    cBefore + 300);
}

TEST(ResourceManagerTest, ApplyEffectSkipsUnspecified) {
    ResourceManager rm;
    LLint before = rm.getResourceValue(WATER);
    rm.applyEffect({{RESOURCE_UNSPECIFIED, 9999}});
    rm.tick();
    EXPECT_EQ(rm.getResourceValue(WATER), before); // unaffected
}

TEST(ResourceManagerTest, ApplyEffectAccumulates) {
    ResourceManager rm;
    rm.applyEffect({{MONEY, 100}});
    rm.applyEffect({{MONEY, 200}});
    LLint before = rm.getResourceValue(MONEY);
    rm.tick();
    EXPECT_EQ(rm.getResourceValue(MONEY), before + 300);
}

// ----------------------------------------------------------------
// ResourceManager – canAfford
// ----------------------------------------------------------------

TEST(ResourceManagerTest, CanAffordWhenSufficientFunds) {
    ResourceManager rm;   // MONEY starts at 250,000
    EXPECT_TRUE(rm.canAfford(100'000LL));
    EXPECT_TRUE(rm.canAfford(250'000LL)); // exact amount
}

TEST(ResourceManagerTest, CannotAffordWhenInsufficientFunds) {
    ResourceManager rm;
    EXPECT_FALSE(rm.canAfford(250'001LL));
    EXPECT_FALSE(rm.canAfford(1'000'000LL));
}

TEST(ResourceManagerTest, CanAffordZero) {
    ResourceManager rm;
    EXPECT_TRUE(rm.canAfford(0));
}

// ----------------------------------------------------------------
// ResourceManager – changeResourceValue
// ----------------------------------------------------------------

TEST(ResourceManagerTest, ChangeResourceValueIncrease) {
    ResourceManager rm;
    LLint before = rm.getResourceValue(CO2);
    rm.changeResourceValue(CO2, 5000);
    EXPECT_EQ(rm.getResourceValue(CO2), before + 5000);
}

TEST(ResourceManagerTest, ChangeResourceValueDecrease) {
    ResourceManager rm;
    LLint before = rm.getResourceValue(POPULATION);
    rm.changeResourceValue(POPULATION, -500'000LL);
    EXPECT_EQ(rm.getResourceValue(POPULATION), before - 500'000LL);
}

TEST(ResourceManagerTest, ChangeResourceValueClampsAtZero) {
    ResourceManager rm;
    rm.changeResourceValue(WATER, -999'999'999LL);
    EXPECT_EQ(rm.getResourceValue(WATER), 0);
}

TEST(ResourceManagerTest, ChangeResourceValueIgnoresUnspecified) {
    ResourceManager rm;
    LLint before = rm.getResourceValue(WATER);
    rm.changeResourceValue(RESOURCE_UNSPECIFIED, 9999);
    EXPECT_EQ(rm.getResourceValue(WATER), before);
}

// ----------------------------------------------------------------
// ResourceManager – iterator
// ----------------------------------------------------------------

TEST(ResourceManagerTest, IteratorCoversAllResources) {
    ResourceManager rm;
    int count = 0;
    for (const Resource& r : rm) { (void)r; ++count; }
    EXPECT_EQ(count, 5);
}

// ----------------------------------------------------------------
// main
// ----------------------------------------------------------------

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}