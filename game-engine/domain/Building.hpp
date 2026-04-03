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
        virtual std::vector<ResourceEffect> createEffects() const = 0;

    protected:
        std::vector<ResourceEffect> effects;

    public:
        Building(enum BuildingType type, int buildCost, int ticksToComplete);

        std::vector<ResourceEffect> buildTick();

        enum BuildingType getType() const;

        int getBuildCost() const;

        int getTicksToComplete() const;

        const std::vector<ResourceEffect>& getEffects() const;

        virtual ~Building() = default;
};

#endif
