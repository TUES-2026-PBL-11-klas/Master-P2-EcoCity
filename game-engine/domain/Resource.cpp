#include "ResourceType.hpp"
#include "Resource.hpp"

Resource::Resource(ResourceType type, int amount, int deltaPerTick)
: type(type), currentValue(amount), deltaPerTick(deltaPerTick) {}

ResourceType Resource::getType() const
{
    return this->type;
}

int Resource::getCurrentValue() const
{
    return this->currentValue;
}

int Resource::getDeltaValue() const
{
    return this->deltaPerTick;
}

void Resource::changeDeltaPerTick(int delta)
{
    this->deltaPerTick += delta;
}

bool Resource::changeCurrentValue()
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
