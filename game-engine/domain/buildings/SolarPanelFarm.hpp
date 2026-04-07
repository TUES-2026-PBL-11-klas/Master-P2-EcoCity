#ifndef SOLAR_PANEL_FARM_H
#define SOLAR_PANEL_FARM_H

#include "../Building.hpp"

class SolarPanelFarm : public Building {
    public:
        SolarPanelFarm();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
