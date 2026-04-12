#include "WindTurbineFarm.hpp"

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
