#ifndef HYDROELECTRIC_PLANT_H
#define HYDROELECTRIC_PLANT_H

#include "../Building.hpp"

constexpr LLint HYDROELECTRIC_COST = 25LL;
constexpr int HYDROELECTRIC_TICKS = 7;
constexpr LLint HYDROELECTRIC_ENERGY = 4'000LL;
constexpr LLint HYDROELECTRIC_CO2 = -190LL;

class HydroelectricPlant : public Building {
    public:
        HydroelectricPlant();
        ~HydroelectricPlant() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
