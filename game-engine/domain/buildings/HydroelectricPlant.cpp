#include "HydroelectricPlant.hpp"

namespace {
constexpr std::int64_t HYDROELECTRIC_COST = 2'500'000LL;
constexpr int HYDROELECTRIC_TICKS = 7;
constexpr std::int64_t HYDROELECTRIC_ENERGY = 4'000'000LL;
constexpr std::int64_t HYDROELECTRIC_CO2 = -950LL;
}

HydroelectricPlant::HydroelectricPlant()
    : Building(HYDROELECTRIC_PLANT, HYDROELECTRIC_COST, HYDROELECTRIC_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> HydroelectricPlant::Effects() const
{
    return {
        { ENERGY, HYDROELECTRIC_ENERGY },
        { CO2, HYDROELECTRIC_CO2 }
    };
}
