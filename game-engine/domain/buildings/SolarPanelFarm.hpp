#ifndef SOLAR_PANEL_FARM_H
#define SOLAR_PANEL_FARM_H

#include "../Building.hpp"

constexpr LLint SOLAR_PANEL_FARM_COST = 10LL;
constexpr int SOLAR_PANEL_FARM_TICKS = 3;
constexpr LLint SOLAR_PANEL_FARM_ENERGY = 850LL;
constexpr LLint SOLAR_PANEL_FARM_CO2 = -38LL;
constexpr LLint SOLAR_PANEL_FARM_WATER = -200LL;

class SolarPanelFarm : public Building {
    public:
        SolarPanelFarm();
        ~SolarPanelFarm() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
