#include "SolarPanelFarm.hpp"

SolarPanelFarm::SolarPanelFarm()
    : Building(SOLAR_PANEL_FARM, SOLAR_PANEL_FARM_COST, SOLAR_PANEL_FARM_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> SolarPanelFarm::Effects() const
{
    return {
        { ENERGY, SOLAR_PANEL_FARM_ENERGY },
        { CO2, SOLAR_PANEL_FARM_CO2 },
        { WATER, SOLAR_PANEL_FARM_WATER }
    };
}
