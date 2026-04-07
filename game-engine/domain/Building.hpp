#ifndef BUILDING_H
#define BUILDING_H

#include <cstdint>
#include <vector>

#include "ResourceEffect.hpp"
#include "BuildingType.hpp"

class Building {
    private:
        enum BuildingType type;
        std::int64_t buildCost;
        int ticksToComplete;

    protected:
        std::vector<ResourceEffect> effects;
        virtual std::vector<ResourceEffect> Effects() const = 0;

    public:
        Building(enum BuildingType type, std::int64_t buildCost, int ticksToComplete);

        std::vector<ResourceEffect> buildTick();

        enum BuildingType getType() const;

        std::int64_t getBuildCost() const;

        int getTicksToComplete() const;

        const std::vector<ResourceEffect>& getEffects() const;

        virtual ~Building() = default;
};

#endif
