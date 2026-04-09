#include "ResourceManager.hpp"

namespace {
std::size_t resourceIndex(ResourceType type)
{
    return static_cast<std::size_t>(type) - 1;
}
}

ResourceManager::ResourceManager()
    : resources{
        Resource(ResourceType::WATER, 1'000'000LL, 0),
        Resource(ResourceType::ENERGY, 5'000'000LL, 0),
        Resource(ResourceType::MONEY, 250'000LL, 0),
        Resource(ResourceType::POPULATION, 1'000'000LL, 0),
        Resource(ResourceType::CO2, 1'000'000LL, 0)
    }
    {
        static_assert(static_cast<int>(ResourceType::WATER)      == 1, "WATER must map to index 0");
        static_assert(static_cast<int>(ResourceType::ENERGY)     == 2, "ENERGY must map to index 1");
        static_assert(static_cast<int>(ResourceType::MONEY)      == 3, "MONEY must map to index 2");
        static_assert(static_cast<int>(ResourceType::POPULATION) == 4, "POPULATION must map to index 3");
        static_assert(static_cast<int>(ResourceType::CO2)        == 5, "CO2 must map to index 4");
    }

bool ResourceManager::tick()
{
    bool gameOver = false;
    for(Resource& resource : resources) {
        if(resource.changeCurrentValue()) {
            gameOver = true;
            break;
        }
    }
    return gameOver;
}

void ResourceManager::applyEffect(const std::vector<ResourceEffect>& effects) {
    for (const ResourceEffect& effect : effects) {
        if (effect.type == ResourceType::RESOURCE_UNSPECIFIED) {
            continue;
        }
        resources[resourceIndex(effect.type)].changeDeltaPerTick(effect.deltaValue);
    }
}

void ResourceManager::changeResourceValue(ResourceType type, std::int64_t delta)
{
    if (type == ResourceType::RESOURCE_UNSPECIFIED) {
        return;
    }
    resources[resourceIndex(type)].changeCurrentValueBy(delta);
}

bool ResourceManager::canAfford(std::int64_t amount) const
{
    return getResourceValue(ResourceType::MONEY) >= amount;
}

std::int64_t ResourceManager::getResourceValue(ResourceType type) const {
    if (type == ResourceType::RESOURCE_UNSPECIFIED) {
        return 0;
    }
    return resources[resourceIndex(type)].getCurrentValue();
}
