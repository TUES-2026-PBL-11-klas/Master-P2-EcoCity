#ifndef ROOFTOP_SOLAR_H
#define ROOFTOP_SOLAR_H

#include "../Building.hpp"

class RooftopSolar : public Building {
    public:
        RooftopSolar();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
