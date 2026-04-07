#ifndef RESOURCE_EFFECT_H
#define RESOURCE_EFFECT_H

#include <cstdint>

#include "ResourceType.hpp"

struct ResourceEffect {
    ResourceType type;
    std::int64_t deltaValue;
};

#endif
