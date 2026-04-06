#ifndef INDUSTRIAL_ZONE_H
#define INDUSTRIAL_ZONE_H

#include "../Building.hpp"

class IndustrialZone : public Building {
    public:
        IndustrialZone();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
