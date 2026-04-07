#include "UrbanGreening.hpp"

namespace {
constexpr std::int64_t URBAN_GREENING_COST = 100'000LL;
constexpr int URBAN_GREENING_TICKS = 2;
constexpr std::int64_t URBAN_GREENING_CO2 = -225LL;
}

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
