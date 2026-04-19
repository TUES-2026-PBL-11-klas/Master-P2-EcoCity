#ifndef ROOFTOP_SOLAR_H
#define ROOFTOP_SOLAR_H

#include "../Building.hpp"

constexpr LLint ROOFTOP_SOLAR_COST = 21LL;
constexpr int ROOFTOP_SOLAR_TICKS = 2;
constexpr LLint ROOFTOP_SOLAR_ENERGY = 750LL;
constexpr LLint ROOFTOP_SOLAR_CO2 = -454LL;
constexpr LLint ROOFTOP_SOLAR_WATER = -2LL;

class RooftopSolar : public Building {
    public:
        RooftopSolar();
        ~RooftopSolar() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
