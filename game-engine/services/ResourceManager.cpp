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
        ResourceManager()
            : water(ResourceType::WATER, 1000, 10),
            energy(ResourceType::ENERGY, 1000, 10),
            co2(ResourceType::CO2, 0, 10),
            money(ResourceType::MONEY, 10000000, 10),
            population(ResourceType::POPULATION, 100, 10)
        {}

        void tick()
        {
            bool gameOver = false;
            if(water.changeCurrentValue() || energy.changeCurrentValue() || co2.changeCurrentValue()
                || money.changeCurrentValue() || population.changeCurrentValue()) {
                    gameOver = true;
                }
        }

        void applyEffect(struct ResourceEffect effect)
        {
            switch (effect.type)
            {
            case ResourceType::WATER:
                water.changeDeltaPerTick(effect.deltaValue);
                break;
            case ResourceType::ENERGY:
                energy.changeDeltaPerTick(effect.deltaValue);
                break;
            case ResourceType::CO2:
                co2.changeDeltaPerTick(effect.deltaValue);
                break;
            case ResourceType::MONEY:
                money.changeDeltaPerTick(effect.deltaValue);
                break;
            case ResourceType::POPULATION:
                population.changeDeltaPerTick(effect.deltaValue);
                break;
            default:
                break;
            }
        }

        int getResourceValue(enum ResourceType type)
        {
            switch (type)
            {
            case ResourceType::WATER:
                return water.getCurrentValue();
            case ResourceType::ENERGY:
                return energy.getCurrentValue();
            case ResourceType::CO2:
                return co2.getCurrentValue();
            case ResourceType::MONEY:
                return money.getCurrentValue();
            case ResourceType::POPULATION:
                return population.getCurrentValue();
            default:
                return 0;
            }
        }
};
