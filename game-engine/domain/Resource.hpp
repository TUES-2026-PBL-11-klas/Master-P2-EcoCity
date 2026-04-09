#ifndef RESOURCE_H
#define RESOURCE_H

#include <cstdint>

#include "ResourceType.hpp"

constexpr std::int64_t MAX_RESOURCE_VALUE = 1'000'000'000LL;

class Resource {
    private:
        ResourceType type;
        std::int64_t currentValue;
        std::int64_t deltaPerTick;
    public:
        Resource(ResourceType type, std::int64_t amount, std::int64_t deltaPerTick);
        ResourceType getType() const;
        std::int64_t getCurrentValue() const;
        std::int64_t getDeltaValue() const;
        void changeDeltaPerTick(std::int64_t delta);
        void changeCurrentValueBy(std::int64_t delta);
        bool changeCurrentValue();
};

#endif
