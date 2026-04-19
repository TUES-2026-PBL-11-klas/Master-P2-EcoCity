#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <vector>
#include <array>
#include <string>
#include "../domain/Resource.hpp"
#include "../domain/ResourceEffect.hpp"

class ResourceManager {
    private:
        std::array<Resource, 5> resources;  // replaces the 5 separate fields, Iterator design pattern

        int getIndexForResourceType(ResourceType type) const;

    public:
        using ResourceArray = std::array<Resource, 5>;

        std::array<Resource, 5>::iterator begin() { return resources.begin(); }
        std::array<Resource, 5>::iterator end()   { return resources.end();   }
        std::array<Resource, 5>::const_iterator begin() const { return resources.begin(); }
        std::array<Resource, 5>::const_iterator end()   const { return resources.end();   }

        ResourceManager();
        void tick(const std::string& traceID="");
        void applyEffect(const std::vector<ResourceEffect>& effects);
        void changeResourceValue(ResourceType type, LLint delta);
        bool canAfford(LLint amount) const;
        LLint getResourceValue(ResourceType type) const;
        const ResourceArray& getResources() const;

        LLint getDeltaForResourceType(ResourceType type) const;
        void setDeltaForResourceType(ResourceType type, LLint delta);
};

#endif
