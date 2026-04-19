#include "WaterSavingInfrastructure.hpp"

WaterSavingInfrastructure::WaterSavingInfrastructure()
    : Building(WATER_SAVING_INFRASTRUCTURE, WATER_SAVING_COST, WATER_SAVING_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> WaterSavingInfrastructure::Effects() const
{
    return {
        { WATER, WATER_SAVING_WATER }
    };
}
