#include "RoadImprovement.hpp"

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
