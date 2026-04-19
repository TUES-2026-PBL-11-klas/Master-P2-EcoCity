#ifndef RESOURCE_EFFECT_H
#define RESOURCE_EFFECT_H

#include "ResourceType.hpp"

typedef long long int LLint;

struct ResourceEffect {
    ResourceType type;
    LLint deltaValue;
};

#endif
