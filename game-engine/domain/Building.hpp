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
        enum BuildingType getType() const;
        int getBuildCost() const;
        int getTicksToComplete() const;
        void changeTicksToComplete(int ticks);
        virtual std::vector<ResourceEffect> applyEffects() const = 0;
};

#endif
