#include "HydroelectricPlant.hpp"

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
