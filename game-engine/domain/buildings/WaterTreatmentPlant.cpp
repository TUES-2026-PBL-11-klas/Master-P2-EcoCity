#include "WaterTreatmentPlant.hpp"

namespace {
constexpr std::int64_t WATER_TREATMENT_COST = 250LL;
constexpr int WATER_TREATMENT_TICKS = 4;
constexpr std::int64_t WATER_TREATMENT_WATER = 36'500LL;
constexpr std::int64_t WATER_TREATMENT_ENERGY = -1'825'000LL;
constexpr std::int64_t WATER_TREATMENT_CO2 = 100LL;
}

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
