#include "ResourceType.hpp"

class Resource {
    private:
        ResourceType type;
        int currentValue;
        int deltaPerTick;
    public:
        Resource(ResourceType type, int amount, int deltaPerTick)
        {
            this->type = type;
            this->currentValue = amount;
            this->deltaPerTick = deltaPerTick;
        }
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
        void setCurrentValue(int value)
        {
            this->currentValue = value;
        }
};