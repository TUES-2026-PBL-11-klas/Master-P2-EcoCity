#ifndef RESOURCE_H
#define RESOURCE_H

#include "ResourceType.hpp"

typedef long long int LLint;

constexpr LLint MAX_RESOURCE_VALUE = 1'000'000'000LL;

class Resource {
    private:
        ResourceType type;
        LLint currentValue;
        LLint deltaPerTick;
    public:
        Resource(ResourceType type, LLint amount, LLint deltaPerTick);
        ResourceType getType() const;
        LLint getCurrentValue() const;
        LLint getDeltaValue() const;
        void changeDeltaPerTick(LLint delta);
        void changeCurrentValue(LLint delta);
        void changeCurrentValue();
};

#endif
