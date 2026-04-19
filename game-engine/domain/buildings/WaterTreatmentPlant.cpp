#include "WaterTreatmentPlant.hpp"

WaterTreatmentPlant::WaterTreatmentPlant()
    : Building(WATER_TREATMENT_PLANT, WATER_TREATMENT_COST, WATER_TREATMENT_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> WaterTreatmentPlant::Effects() const
{
    return {
        { WATER, WATER_TREATMENT_WATER },
        { ENERGY, WATER_TREATMENT_ENERGY },
        { CO2, WATER_TREATMENT_CO2 }
    };
}
