#ifndef WATER_TREATMENT_PLANT_H
#define WATER_TREATMENT_PLANT_H

#include "../Building.hpp"

constexpr LLint WATER_TREATMENT_COST = 250LL;
constexpr int WATER_TREATMENT_TICKS = 4;
constexpr LLint WATER_TREATMENT_WATER = 36'500LL;
constexpr LLint WATER_TREATMENT_ENERGY = -1'825'000LL;
constexpr LLint WATER_TREATMENT_CO2 = 100LL;

class WaterTreatmentPlant : public Building {
    public:
        WaterTreatmentPlant();
        ~WaterTreatmentPlant() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
