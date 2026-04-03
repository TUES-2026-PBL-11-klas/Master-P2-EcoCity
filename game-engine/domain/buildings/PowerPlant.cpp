#include "PowerPlant.hpp"

PowerPlant::PowerPlant()
    : Building(BuildingType::POWER_PLANT, POWER_PLANT_COST, POWER_PLANT_TICKS) {}

std::vector<ResourceEffect> PowerPlant::applyEffects() const {
    return {
        {ENERGY, POWER_PLANT_ENERGY_EFFECT},
        {CO2, POWER_PLANT_CO2_EFFECT}
    };
}
