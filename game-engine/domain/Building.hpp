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

    public:
        Building(enum BuildingType type, int buildCost, int ticksToComplete, std::vector<ResourceEffect> effects);

        enum BuildingType getType() const
        {
            return type;
        };

        int getBuildCost() const
        {
            return buildCost;
        };

        int getTicksToComplete() const
        {
            return ticksToComplete;
        };

        void changeTicksToComplete(int ticks)
        {
            ticksToComplete += ticks;
        };

        virtual std::vector<ResourceEffect> applyEffects() const = 0;
};

#endif
