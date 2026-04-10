#ifndef POWER_PLANT_H
#define POWER_PLANT_H

#include "../Building.hpp"

// To make it easier to change later, might be best to move to a config file
#define POWER_PLANT_COST 500
#define POWER_PLANT_TICKS 10
#define POWER_PLANT_ENERGY_EFFECT 30
#define POWER_PLANT_CO2_EFFECT 15

// This is a child example, it currently has fixed values
// ADD randomization of values
class PowerPlant : public Building {
    public:
        PowerPlant();
        ~PowerPlant() override = default;

    private:
        std::vector<ResourceEffect> createEffects() const override;

};

#endif
