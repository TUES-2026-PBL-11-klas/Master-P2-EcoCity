#include "AirportExpansion.hpp"

AirportExpansion::AirportExpansion()
    : Building(AIRPORT_EXPANSION, AIRPORT_EXPANSION_COST, AIRPORT_EXPANSION_TICKS)
    {
        effects = Effects();
    }

std::vector<ResourceEffect> AirportExpansion::Effects() const
{
    return {
        { MONEY, AIRPORT_EXPANSION_MONEY },
        { CO2, AIRPORT_EXPANSION_CO2 },
        { ENERGY, AIRPORT_EXPANSION_ENERGY },
        { WATER, AIRPORT_EXPANSION_WATER }
    };
}
