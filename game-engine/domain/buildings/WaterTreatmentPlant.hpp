#ifndef WATER_TREATMENT_PLANT_H
#define WATER_TREATMENT_PLANT_H

#include "../Building.hpp"

class WaterTreatmentPlant : public Building {
    public:
        WaterTreatmentPlant();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
