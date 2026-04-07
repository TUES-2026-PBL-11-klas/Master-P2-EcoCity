#include "SolarPanelFarm.hpp"

namespace {
constexpr std::int64_t SOLAR_PANEL_FARM_COST = 1'000'000LL;
constexpr int SOLAR_PANEL_FARM_TICKS = 3;
constexpr std::int64_t SOLAR_PANEL_FARM_ENERGY = 850'000LL;
constexpr std::int64_t SOLAR_PANEL_FARM_CO2 = -190LL;
constexpr std::int64_t SOLAR_PANEL_FARM_WATER = -200'000LL;
}

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
