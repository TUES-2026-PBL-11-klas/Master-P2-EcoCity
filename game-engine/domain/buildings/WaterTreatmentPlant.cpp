#include "WaterTreatmentPlant.hpp"

namespace {
constexpr std::int64_t WATER_TREATMENT_COST = 25'000'000LL;
constexpr int WATER_TREATMENT_TICKS = 4;
constexpr std::int64_t WATER_TREATMENT_WATER = 36'500'000LL;
constexpr std::int64_t WATER_TREATMENT_ENERGY = -1'825'000'000LL;
constexpr std::int64_t WATER_TREATMENT_CO2 = 500LL;
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
