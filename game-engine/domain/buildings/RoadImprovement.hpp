#ifndef ROAD_IMPROVEMENT_H
#define ROAD_IMPROVEMENT_H

#include "../Building.hpp"

constexpr LLint ROAD_IMPROVEMENT_COST = 500LL;
constexpr int ROAD_IMPROVEMENT_TICKS = 4;
constexpr LLint ROAD_IMPROVEMENT_MONEY = 100LL;
constexpr LLint ROAD_IMPROVEMENT_CO2 = 5'200LL;

class RoadImprovement : public Building {
    public:
        RoadImprovement();
        ~RoadImprovement() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
