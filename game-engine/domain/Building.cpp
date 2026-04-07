#include "Building.hpp"

Building::Building(enum BuildingType type, std::int64_t buildCost, int ticksToComplete)
: type(type), buildCost(buildCost), ticksToComplete(ticksToComplete) {}

std::vector<ResourceEffect> Building::buildTick()
{
    ticksToComplete--;
    if (ticksToComplete > 0) {
        return {};              // not done yet, no effects
    }
    return effects;             // done, subclass gives the effects
}

enum BuildingType Building::getType() const
{
    return type;
}

std::int64_t Building::getBuildCost() const
{
    return buildCost;
}

int Building::getTicksToComplete() const
{
    return ticksToComplete;
}

const std::vector<ResourceEffect>& Building::getEffects() const
{
    return effects;
}
