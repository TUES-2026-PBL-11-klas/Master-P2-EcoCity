#ifndef BUILDING_H
#define BUILDING_H

#include <string>
#include <vector>

#include "ResourceEffect.hpp"
#include "BuildingType.hpp"

// Building class is supposed to be abstract, but it is not currently implemented as such

class Building {
    private:
        enum BuildingType type;
        int buildCost;
        int ticksToComplete;
        std::vector<ResourceEffect> effects;

    public:
        Building(enum BuildingType type, int buildCost, int ticksToComplete, std::vector<ResourceEffect> effects);
        enum BuildingType getType() const;
        int getBuildCost() const;
        int getTicksToComplete() const;
        const std::vector<ResourceEffect>& getEffects() const;
        ResourceEffect applyEffects() const; // Idk if this method won't be the same as the getEffects() method
};

#endif
