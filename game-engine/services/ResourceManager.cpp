#include "ResourceManager.hpp"

#include <iostream>

// Enum order:  WATER=0, ENERGY=1, MONEY=2, POPULATION=3, CO2=4
// Array must follow the same order:
ResourceManager::ResourceManager()
    : resources{
        Resource(ResourceType::WATER, 1'000'000LL, -10000),          // index 0
        Resource(ResourceType::ENERGY, 5'000'000LL, -10000),         // index 1
        Resource(ResourceType::MONEY, 250'000LL, +10000),            // index 2
        Resource(ResourceType::POPULATION, 1'000'000LL, +10000),     // index 3
        Resource(ResourceType::CO2, 1'000'000LL, +10000)             // index 4
    }
    {
        // Static assertions to ensure enum values match array indices
        // static_assert checks happen at compile time so there is zero runtime cost
        static_assert(static_cast<int>(ResourceType::WATER)      == 1, "WATER must be index 0 so in the enum it must be 1");
        static_assert(static_cast<int>(ResourceType::ENERGY)     == 2, "ENERGY must be index 1");
        static_assert(static_cast<int>(ResourceType::MONEY)      == 3, "MONEY must be index 2");
        static_assert(static_cast<int>(ResourceType::POPULATION) == 4, "POPULATION must be index 3");
        static_assert(static_cast<int>(ResourceType::CO2)        == 5, "CO2 must be index 4");
    }

int ResourceManager::getIndexForResourceType(ResourceType type) const
{
    return static_cast<int>(type) - 1; // enum starts at 1 for WATER, but WATER is at index 0 in arr
}

void ResourceManager::tick()
{
    for(Resource& resource : resources)
    {
        resource.changeCurrentValue();
    }
}

void ResourceManager::applyEffect(const std::vector<ResourceEffect>& effects) {
    for (const ResourceEffect& effect : effects) {
        if (effect.type == ResourceType::RESOURCE_UNSPECIFIED) {
            continue;
        }
        resources[getIndexForResourceType(effect.type)].changeDeltaPerTick(effect.deltaValue);
    }
}

LLint ResourceManager::getResourceValue(ResourceType type) const {
    return resources[getIndexForResourceType(type)].getCurrentValue();
}

bool ResourceManager::canAfford(LLint amount) const
{
    return getResourceValue(ResourceType::MONEY) >= amount;
}

void ResourceManager::changeResourceValue(ResourceType type, LLint delta)
{
    if (type == ResourceType::RESOURCE_UNSPECIFIED) {
        return;
    }
    resources[getIndexForResourceType(type)].changeCurrentValue(delta);
}

const ResourceManager::ResourceArray& ResourceManager::getResources() const
{
    return resources;
}

LLint ResourceManager::getDeltaForResourceType(ResourceType type) const
{
    return resources[getIndexForResourceType(type)].getDeltaValue();
}

void ResourceManager::setDeltaForResourceType(ResourceType type, LLint delta)
{
    resources[getIndexForResourceType(type)].setDeltaPerTick(delta);
}
