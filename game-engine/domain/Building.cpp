#include "Building.hpp"

Building::Building(enum BuildingType type, int buildCost, int ticksToComplete)
: type(type), buildCost(buildCost), ticksToComplete(ticksToComplete) {}

std::vector<ResourceEffect> Building::buildTick()
{
    ticksToComplete--;
    if (ticksToComplete > 0) {
        return {};              // not done yet, no effects
    }
    return applyEffects();      // done, subclass gives the effects
}

enum BuildingType Building::getType() const
{
    return type;
}

int Building::getBuildCost() const
{
    return buildCost;
}

int Building::getTicksToComplete() const
{
    return ticksToComplete;
}
