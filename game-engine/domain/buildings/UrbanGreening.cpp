#include "UrbanGreening.hpp"

UrbanGreening::UrbanGreening()
    : Building(URBAN_GREENING, URBAN_GREENING_COST, URBAN_GREENING_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> UrbanGreening::Effects() const
{
    return {
        { CO2, URBAN_GREENING_CO2 }
    };
}
