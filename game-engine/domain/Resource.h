#ifndef RESOURCE_H
#define RESOURCE_H

#include "ResourceType.h"


class Resource {
    private:
    ResourceType type;
    int currentValue;
    int deltaPerTick;
public:
    Resource(ResourceType type, int amount);
    ResourceType getType() const;
    int getCurrentValue() const;
    int getDeltaValue() const;
};

#endif