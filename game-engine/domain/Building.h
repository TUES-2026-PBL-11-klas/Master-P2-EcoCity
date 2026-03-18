#ifndef BUILDING_H
#define BUILDING_H

#include <string>
#include <vector>
#include "ResourceEffect.h"


class Building {
    private:
        std::string name;
        int buildCost;
        std::vector<ResourceEffect> effects;

public:
    Building(std::string name, int buildCost, std::vector<ResourceEffect> effects);
    const std::string& getName() const;
    int getBuildCost() const;
    const std::vector<ResourceEffect>& getEffects() const;
    ResourceEffect applyEffects() const;
};


#endif