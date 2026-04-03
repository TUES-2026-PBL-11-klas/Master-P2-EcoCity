#ifndef BUILDING_H
#define BUILDING_H

#include <string>
#include <vector>

#include "ResourceEffect.hpp"
#include "BuildingType.hpp"

class Building {
    private:
        enum BuildingType type;
        int buildCost;
        int ticksToComplete;

        // Subclasses implement this, but it can't be called directly
        virtual std::vector<ResourceEffect> applyEffects() const = 0;

    public:
        Building(enum BuildingType type, int buildCost, int ticksToComplete)
        {
            this->type = type;
            this->buildCost = buildCost;
            this->ticksToComplete = ticksToComplete;
        }

        std::vector<ResourceEffect> buildTick()
        {
            ticksToComplete--;
            if (ticksToComplete > 0) {
                return {};              // not done yet, no effects
            }
            return applyEffects();      // done, subclass gives the effects
        }

        enum BuildingType getType() const
        {
            return type;
        }

        int getBuildCost() const
        {
            return buildCost;
        }

        int getTicksToComplete() const
        {
            return ticksToComplete;
        }

        std::vector<ResourceEffect> getEffects() const
        {
            return applyEffects();
        }
};

#endif
