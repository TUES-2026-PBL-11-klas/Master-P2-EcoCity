#include "WaterSavingInfrastructure.hpp"

namespace {
constexpr std::int64_t WATER_SAVING_COST = 500'000'000LL;
constexpr int WATER_SAVING_TICKS = 5;
constexpr std::int64_t WATER_SAVING_WATER = 150'000'000LL;
}

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
