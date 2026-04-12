#include "IndustrialZone.hpp"

IndustrialZone::IndustrialZone()
    : Building(INDUSTRIAL_ZONE, INDUSTRIAL_ZONE_COST, INDUSTRIAL_ZONE_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> IndustrialZone::Effects() const
{
    return {
        { MONEY, INDUSTRIAL_ZONE_MONEY },
        { CO2, INDUSTRIAL_ZONE_CO2 }
    };
}
