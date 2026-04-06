#ifndef WIND_TURBINE_FARM_H
#define WIND_TURBINE_FARM_H

#include "../Building.hpp"

class WindTurbineFarm : public Building {
    public:
        WindTurbineFarm();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
