#ifndef RESOURCE_H
#define RESOURCE_H

#include "ResourceType.hpp"

#define MAX_RESOURCE_VALUE 65536

class Resource {
    private:
        ResourceType type;
        int currentValue;
        int deltaPerTick;
    public:
        Resource(ResourceType type, int amount, int deltaPerTick);
        ResourceType getType() const;
        int getCurrentValue() const;
        int getDeltaValue() const;
        void changeDeltaPerTick(int delta);
        void changeCurrentValue();
};

#endif
