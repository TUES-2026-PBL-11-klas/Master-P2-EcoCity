#ifndef PETITION_H
#define PETITION_H

#include "ResourceEffect.h"
#include <string>
#include <vector>

class Petition{
    private:
        std::string title;
        std::string description;
        int cost;
        int ticksToComplete;
        std::vector<ResourceEffect> effects;
        
    public:
        Petition(std::string title, std::string description, int cost, int ticksToComplete, std::vector<ResourceEffect> effects);
        const std::string& getTitle() const;
        const std::string& getDescription() const;
        int getCost() const;
        int getTicksToComplete() const;
        const std::vector<ResourceEffect>& getEffects() const;
};
#endif