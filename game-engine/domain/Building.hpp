#ifndef BUILDING_H
#define BUILDING_H

#include <vector>

#include "ResourceEffect.hpp"
#include "BuildingType.hpp"

typedef long long int LLint;

class Building {
    private:
        enum BuildingType type;
        LLint buildCost;
        int ticksToComplete;

    protected:
        std::vector<ResourceEffect> effects;

        // Subclasses implement this, but it can't be called directly
        virtual std::vector<ResourceEffect> Effects() const = 0;

    public:
        Building(enum BuildingType type, LLint buildCost, int ticksToComplete);

        std::vector<ResourceEffect> buildTick();

        enum BuildingType getType() const;

        LLint getBuildCost() const;

        int getTicksToComplete() const;

        const std::vector<ResourceEffect>& getEffects() const;

        void setTicksToComplete(int ticks);
        void setBuildCost(LLint cost);

        virtual ~Building() = default;
};

#endif
