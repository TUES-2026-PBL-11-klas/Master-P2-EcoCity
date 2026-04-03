#ifndef BUILDING_H
#define BUILDING_H

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
        Building(enum BuildingType type, int buildCost, int ticksToComplete);

        std::vector<ResourceEffect> buildTick();

        enum BuildingType getType() const;

        int getBuildCost() const;

        int getTicksToComplete() const;

        virtual ~Building() = default;
};

#endif
