#ifndef URBAN_GREENING_H
#define URBAN_GREENING_H

#include "../Building.hpp"

constexpr LLint URBAN_GREENING_COST = 1LL;
constexpr int URBAN_GREENING_TICKS = 2;
constexpr LLint URBAN_GREENING_CO2 = -45LL;

class UrbanGreening : public Building {
    public:
        UrbanGreening();
        ~UrbanGreening() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
