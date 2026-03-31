#include "domain/Resource.hpp"
#include "domain/ResourceEffect.hpp"
#include "services/ResourceManager.hpp"

class ResourceManager {
    private:
        std::array<Resource, 5> resources;

    public:
        // Enum order:  WATER=0, ENERGY=1, MONEY=2, POPULATION=3, CO2=4
        // Array must follow the same order:
        ResourceManager()
            : resources{
                Resource(ResourceType::WATER, 1000, 10),        // index 0
                Resource(ResourceType::ENERGY, 1000, 10),       // index 1
                Resource(ResourceType::MONEY, 10000000, 10),    // index 2
                Resource(ResourceType::POPULATION, 100, 10),    // index 3
                Resource(ResourceType::CO2, 1000, 10)           // index 4
            }
            {
                // Static assertions to ensure enum values match array indices
                // static_assert checks happen at compile time so there is zero runtime cost
                static_assert(static_cast<int>(ResourceType::WATER)      == 0, "WATER must be index 0");
                static_assert(static_cast<int>(ResourceType::ENERGY)     == 1, "ENERGY must be index 1");
                static_assert(static_cast<int>(ResourceType::MONEY)      == 2, "MONEY must be index 2");
                static_assert(static_cast<int>(ResourceType::POPULATION) == 3, "POPULATION must be index 3");
                static_assert(static_cast<int>(ResourceType::CO2)        == 4, "CO2 must be index 4");
            }

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

        void applyEffect(const std::vector<ResourceEffect>& effects) {
            for (const ResourceEffect& effect : effects) {
                resources[static_cast<int>(effect.type)].changeDeltaPerTick(effect.deltaValue);
            }
        }

        int getResourceValue(ResourceType type) {
            return resources[static_cast<int>(type)].getCurrentValue();
        }
};
