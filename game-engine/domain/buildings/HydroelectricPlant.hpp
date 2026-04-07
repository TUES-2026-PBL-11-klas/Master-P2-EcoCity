#ifndef HYDROELECTRIC_PLANT_H
#define HYDROELECTRIC_PLANT_H

#include "../Building.hpp"

class HydroelectricPlant : public Building {
    public:
        HydroelectricPlant();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
