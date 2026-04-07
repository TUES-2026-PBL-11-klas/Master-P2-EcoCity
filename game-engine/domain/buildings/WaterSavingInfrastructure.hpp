#ifndef WATER_SAVING_INFRASTRUCTURE_H
#define WATER_SAVING_INFRASTRUCTURE_H

#include "../Building.hpp"

class WaterSavingInfrastructure : public Building {
    public:
        WaterSavingInfrastructure();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
