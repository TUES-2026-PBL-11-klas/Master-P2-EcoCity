#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "domain/Resource.hpp"
#include "domain/ResourceEffect.hpp"

class ResourceManager {
    private:
        Resource water;
        Resource energy;
        Resource co2;
        Resource money;
        Resource population;
    public:
        ResourceManager();
        void tick();
        void applyEffect(struct ResourceEffect effect);
        int getResourceValue(enum ResourceType type);
};

#endif
