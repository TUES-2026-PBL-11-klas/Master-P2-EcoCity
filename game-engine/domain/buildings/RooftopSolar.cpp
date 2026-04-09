#include "RooftopSolar.hpp"

namespace {
constexpr std::int64_t ROOFTOP_SOLAR_COST = 21LL;
constexpr int ROOFTOP_SOLAR_TICKS = 2;
constexpr std::int64_t ROOFTOP_SOLAR_ENERGY = 750LL;
constexpr std::int64_t ROOFTOP_SOLAR_CO2 = -454LL;
constexpr std::int64_t ROOFTOP_SOLAR_WATER = -2LL;
}

RooftopSolar::RooftopSolar()
    : Building(SOLAR_PANEL_ROOFTOPS, ROOFTOP_SOLAR_COST, ROOFTOP_SOLAR_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> RooftopSolar::Effects() const
{
    return {
        { ENERGY, ROOFTOP_SOLAR_ENERGY },
        { CO2, ROOFTOP_SOLAR_CO2 },
        { WATER, ROOFTOP_SOLAR_WATER }
    };
}
