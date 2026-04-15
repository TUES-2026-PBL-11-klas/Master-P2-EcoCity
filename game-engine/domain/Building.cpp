#include "Building.hpp"

Building::Building(enum BuildingType type, LLint buildCost, int ticksToComplete)
: type(type), buildCost(buildCost), ticksToComplete(ticksToComplete) {}

std::vector<ResourceEffect> Building::buildTick()
{
    if(ticksToComplete <= 0) {
        return {};              // Already completed, no effects, in case this is called again by mistake
    }
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

LLint Building::getBuildCost() const
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

void Building::setTicksToComplete(int ticks)
{
    if(ticks < 0) ticks = 0;
    this->ticksToComplete = ticks;
}

void Building::setBuildCost(LLint cost)
{
    if(cost < 0) cost = 0;
    this->buildCost = cost;
}
