#ifndef POWER_PLANT_H
#define POWER_PLANT_H

#include "../Building.hpp"

// To make it easier to change later, might be best to move to a config file
#define POWER_PLANT_COST 900LL
#define POWER_PLANT_TICKS 7
#define POWER_PLANT_ENERGY_EFFECT 100LL
#define POWER_PLANT_CO2_EFFECT 150LL

// This is a child example, it currently has fixed values
// ADD randomization of values
class PowerPlant : public Building {
    public:
        PowerPlant();
        ~PowerPlant() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;

};

#endif
