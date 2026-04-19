#include "Resource.hpp"

Resource::Resource(ResourceType type, LLint amount, LLint deltaPerTick)
: type(type), currentValue(amount), deltaPerTick(deltaPerTick) {}

ResourceType Resource::getType() const
{
    return this->type;
}

LLint Resource::getCurrentValue() const
{
    return this->currentValue;
}

LLint Resource::getDeltaValue() const
{
    return this->deltaPerTick;
}

void Resource::setDeltaPerTick(LLint delta)
{
    this->deltaPerTick = delta;
}

void Resource::changeDeltaPerTick(LLint delta)
{
    this->deltaPerTick += delta;
}

void Resource::changeCurrentValue(LLint delta)
{
    this->currentValue += delta;
    if (this->currentValue < 0) {
        this->currentValue = 0;
    }
    if (this->currentValue > MAX_RESOURCE_VALUE) {
        this->currentValue = MAX_RESOURCE_VALUE;
    }
}

void Resource::changeCurrentValue()
{
    this->currentValue += this->deltaPerTick;
    if (this->currentValue < 0) {
        this->currentValue = 0;
    }
    if (this->currentValue > MAX_RESOURCE_VALUE) {
        this->currentValue = MAX_RESOURCE_VALUE;
    }
}
