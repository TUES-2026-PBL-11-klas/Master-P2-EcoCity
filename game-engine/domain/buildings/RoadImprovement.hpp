#ifndef ROAD_IMPROVEMENT_H
#define ROAD_IMPROVEMENT_H

#include "../Building.hpp"

class RoadImprovement : public Building {
    public:
        RoadImprovement();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
