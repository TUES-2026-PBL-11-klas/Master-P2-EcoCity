#include "domain/Resource.hpp"
#include "domain/ResourceEffect.hpp"
#include "services/ResourceManager.hpp"

class ResourceManager {
    private:
        std::array<Resource, 5> resources;

    public:
        ResourceManager()
            : resources{
                Resource(ResourceType::WATER, 1000, 10),
                Resource(ResourceType::ENERGY, 1000, 10),
                Resource(ResourceType::CO2, 1000, 10),
                Resource(ResourceType::MONEY, 10000000, 10),
                Resource(ResourceType::POPULATION, 100, 10)
            } {}

        bool tick()
        {
            bool gameOver = false;
            for(Resource& resource : resources) {
                if(resource.changeCurrentValue()) {
                    gameOver = true;
                    break;
                }
            }
            return gameOver;
        }

        void applyEffect(struct ResourceEffect effect)
        {
            for(Resource& resource : resources) {
                if(resource.getType() == effect.type) {
                    resource.changeDeltaPerTick(effect.deltaValue);
                    break;
                }
            }
        }

        int getResourceValue(enum ResourceType type)
        {
            for(Resource& resource : resources) {
                if(resource.getType() == type) {
                    return resource.getCurrentValue();
                }
            }

            return -1; // Return -1 if resource type not found
        }
};
