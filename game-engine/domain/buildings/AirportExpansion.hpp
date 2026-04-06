#ifndef AIRPORT_EXPANSION_H
#define AIRPORT_EXPANSION_H

#include "../Building.hpp"

class AirportExpansion : public Building {
    public:
        AirportExpansion();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
