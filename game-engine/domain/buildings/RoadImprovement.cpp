#include "RoadImprovement.hpp"

namespace {
constexpr std::int64_t ROAD_IMPROVEMENT_COST = 50'000'000LL;
constexpr int ROAD_IMPROVEMENT_TICKS = 4;
constexpr std::int64_t ROAD_IMPROVEMENT_MONEY = 10'000'000LL;
constexpr std::int64_t ROAD_IMPROVEMENT_CO2 = 26'000LL;
}

RoadImprovement::RoadImprovement()
    : Building(ROAD_IMPROVEMENT, ROAD_IMPROVEMENT_COST, ROAD_IMPROVEMENT_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> RoadImprovement::Effects() const
{
    return {
        { MONEY, ROAD_IMPROVEMENT_MONEY },
        { CO2, ROAD_IMPROVEMENT_CO2 }
    };
}
