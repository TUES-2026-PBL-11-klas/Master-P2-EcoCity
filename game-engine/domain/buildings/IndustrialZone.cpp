#include "IndustrialZone.hpp"

namespace {
constexpr std::int64_t INDUSTRIAL_ZONE_COST = 35'000LL;
constexpr int INDUSTRIAL_ZONE_TICKS = 6;
constexpr std::int64_t INDUSTRIAL_ZONE_MONEY = 6'600LL;
constexpr std::int64_t INDUSTRIAL_ZONE_CO2 = 100'000LL;
}

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
