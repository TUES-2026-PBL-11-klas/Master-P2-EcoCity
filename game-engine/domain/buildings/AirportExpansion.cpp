#include "AirportExpansion.hpp"

namespace {
constexpr std::int64_t AIRPORT_EXPANSION_COST = 5'000LL;
constexpr int AIRPORT_EXPANSION_TICKS = 5;
constexpr std::int64_t AIRPORT_EXPANSION_MONEY = 1'000LL;
constexpr std::int64_t AIRPORT_EXPANSION_CO2 = 20'000LL;
constexpr std::int64_t AIRPORT_EXPANSION_ENERGY = -30'000LL;
constexpr std::int64_t AIRPORT_EXPANSION_WATER = -80'000LL;
}

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
