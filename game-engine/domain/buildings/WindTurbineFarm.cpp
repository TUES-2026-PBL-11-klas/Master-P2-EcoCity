#include "WindTurbineFarm.hpp"

namespace {
constexpr std::int64_t WIND_TURBINE_COST = 21'000'000LL;
constexpr int WIND_TURBINE_TICKS = 5;
constexpr std::int64_t WIND_TURBINE_ENERGY = 460'000'000LL;
constexpr std::int64_t WIND_TURBINE_CO2 = -700LL;
}

WindTurbineFarm::WindTurbineFarm()
    : Building(WIND_TURBINE_FARM, WIND_TURBINE_COST, WIND_TURBINE_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> WindTurbineFarm::Effects() const
{
    return {
        { ENERGY, WIND_TURBINE_ENERGY },
        { CO2, WIND_TURBINE_CO2 }
    };
}
