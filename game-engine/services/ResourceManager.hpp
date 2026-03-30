#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <array>
#include "domain/Resource.hpp"
#include "domain/ResourceEffect.hpp"

class ResourceManager {
    private:
        std::array<Resource, 5> resources;  // replaces the 5 separate fields, Iterator design pattern

    public:
        std::array<Resource, 5>::iterator begin() { return resources.begin(); }
        std::array<Resource, 5>::iterator end()   { return resources.end();   }

        ResourceManager();
        bool tick();
        void applyEffect(struct ResourceEffect effect);
        int getResourceValue(enum ResourceType type);
};

#endif
