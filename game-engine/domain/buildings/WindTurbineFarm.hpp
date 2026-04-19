#ifndef WIND_TURBINE_FARM_H
#define WIND_TURBINE_FARM_H

#include "../Building.hpp"

constexpr LLint WIND_TURBINE_COST = 210LL;
constexpr int WIND_TURBINE_TICKS = 5;
constexpr LLint WIND_TURBINE_ENERGY = 460'000LL;
constexpr LLint WIND_TURBINE_CO2 = -140LL;

class WindTurbineFarm : public Building {
    public:
        WindTurbineFarm();
        ~WindTurbineFarm() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
