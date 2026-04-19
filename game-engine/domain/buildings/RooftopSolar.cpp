#include "RooftopSolar.hpp"

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
