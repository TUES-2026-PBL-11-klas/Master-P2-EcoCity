#include "ResourceType.hpp"
#include "Resource.hpp"

class Resource {
    private:
        ResourceType type;
        int currentValue;
        int deltaPerTick;
    public:
        Resource(ResourceType type, int amount, int deltaPerTick)
        : type(type), currentValue(amount), deltaPerTick(deltaPerTick) {}

        ResourceType getType() const
        {
            return this->type;
        }
        int getCurrentValue() const
        {
            return this->currentValue;
        }
        int getDeltaValue() const
        {
            return this->deltaPerTick;
        }
        void changeDeltaPerTick(int delta)
        {
            this->deltaPerTick += delta;
        }
        bool changeCurrentValue()
        {
            this->currentValue += this->deltaPerTick;
            if (this->currentValue < 0) {
                this->currentValue = 0;
                return true; // Game over
            }
            if (this->currentValue > MAX_RESOURCE_VALUE) {
                this->currentValue = MAX_RESOURCE_VALUE;
            }
            return false;
        }
};
