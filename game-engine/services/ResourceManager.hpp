#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <vector>
#include <array>
#include "../domain/Resource.hpp"
#include "../domain/ResourceEffect.hpp"

class ResourceManager {
    private:
        std::array<Resource, 5> resources;  // replaces the 5 separate fields, Iterator design pattern

        int getIndexForResourceType(ResourceType type) const;

    public:
        std::array<Resource, 5>::iterator begin() { return resources.begin(); }
        std::array<Resource, 5>::iterator end()   { return resources.end();   }

        ResourceManager();
        void tick();
        void applyEffect(const std::vector<ResourceEffect>& effects);
        void changeResourceValue(ResourceType type, LLint delta);
        bool canAfford(LLint amount) const;
        int getResourceValue(enum ResourceType type) const;
};

#endif
