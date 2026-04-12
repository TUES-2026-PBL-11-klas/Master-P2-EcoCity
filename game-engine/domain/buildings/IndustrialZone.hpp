#ifndef INDUSTRIAL_ZONE_H
#define INDUSTRIAL_ZONE_H

#include "../Building.hpp"

constexpr LLint INDUSTRIAL_ZONE_COST = 35'000LL;
constexpr int INDUSTRIAL_ZONE_TICKS = 6;
constexpr LLint INDUSTRIAL_ZONE_MONEY = 6'600LL;
constexpr LLint INDUSTRIAL_ZONE_CO2 = 100'000LL;

class IndustrialZone : public Building {
    public:
        IndustrialZone();
        ~IndustrialZone() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
