#include "Resource.hpp"

Resource::Resource(ResourceType type, std::int64_t amount, std::int64_t deltaPerTick)
: type(type), currentValue(amount), deltaPerTick(deltaPerTick) {}

ResourceType Resource::getType() const
{
    return this->type;
}

std::int64_t Resource::getCurrentValue() const
{
    return this->currentValue;
}

std::int64_t Resource::getDeltaValue() const
{
    return this->deltaPerTick;
}

void Resource::changeDeltaPerTick(std::int64_t delta)
{
    this->deltaPerTick += delta;
}

void Resource::changeCurrentValueBy(std::int64_t delta)
{
    this->currentValue += delta;
    if (this->currentValue < 0) {
        this->currentValue = 0;
    }
    if (this->currentValue > MAX_RESOURCE_VALUE) {
        this->currentValue = MAX_RESOURCE_VALUE;
    }
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
