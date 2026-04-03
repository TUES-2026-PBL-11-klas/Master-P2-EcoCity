#include "ResourceManager.hpp"

// Enum order:  WATER=0, ENERGY=1, MONEY=2, POPULATION=3, CO2=4
// Array must follow the same order:
ResourceManager::ResourceManager()
    : resources{
        Resource(ResourceType::WATER, 1000, 10),        // index 0
        Resource(ResourceType::ENERGY, 1000, 10),       // index 1
        Resource(ResourceType::MONEY, 10000000, 10),    // index 2
        Resource(ResourceType::POPULATION, 100, 10),    // index 3
        Resource(ResourceType::CO2, 1000, 10)           // index 4
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
    return static_cast<int>(type) - 1; // enum starts at 1 for WATER, but WATER is at index 0
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
        resources[getIndexForResourceType(effect.type)].changeDeltaPerTick(effect.deltaValue);
    }
}

int ResourceManager::getResourceValue(ResourceType type) {
    return resources[getIndexForResourceType(type)].getCurrentValue();
}
