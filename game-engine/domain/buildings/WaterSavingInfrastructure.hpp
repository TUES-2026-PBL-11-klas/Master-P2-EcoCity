#ifndef WATER_SAVING_INFRASTRUCTURE_H
#define WATER_SAVING_INFRASTRUCTURE_H

#include "../Building.hpp"

constexpr LLint WATER_SAVING_COST = 5'000LL;
constexpr int WATER_SAVING_TICKS = 5;
constexpr LLint WATER_SAVING_WATER = 150'000LL;

class WaterSavingInfrastructure : public Building {
    public:
        WaterSavingInfrastructure();
        ~WaterSavingInfrastructure() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
