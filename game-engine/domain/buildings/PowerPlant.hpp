#ifndef POWER_PLANT_H
#define POWER_PLANT_H

#include "domain/Building.hpp"

// To make it easier to change later
#define POWER_PLANT_COST 500
#define POWER_PLANT_TICKS 10
#define POWER_PLANT_ENERGY_EFFECT 30
#define POWER_PLANT_CO2_EFFECT 15

// This is a child example, it currently has fixed values
// ADD randomization of values
class PowerPlant : public Building {
    private:
        std::vector<ResourceEffect> applyEffects() const override {
            return {
                {ENERGY, POWER_PLANT_ENERGY_EFFECT},
                {CO2, POWER_PLANT_CO2_EFFECT}
            };
        }

    public:
        PowerPlant() : Building(BuildingType::POWER_PLANT, POWER_PLANT_COST, POWER_PLANT_TICKS) {}

};

#endif
