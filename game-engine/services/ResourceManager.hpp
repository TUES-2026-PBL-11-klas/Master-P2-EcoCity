#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <array>
#include <cstdint>
#include <vector>

#include "../domain/Resource.hpp"
#include "../domain/ResourceEffect.hpp"

class ResourceManager {
    private:
        std::array<Resource, 5> resources;  // replaces the 5 separate fields, Iterator design pattern

    public:
        std::array<Resource, 5>::iterator begin() { return resources.begin(); }
        std::array<Resource, 5>::iterator end()   { return resources.end();   }

        ResourceManager();
        bool tick();
        void applyEffect(const std::vector<ResourceEffect>& effects);
        void changeResourceValue(ResourceType type, std::int64_t delta);
        bool canAfford(std::int64_t amount) const;
        std::int64_t getResourceValue(ResourceType type) const;
};

#endif
