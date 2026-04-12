#ifndef AIRPORT_EXPANSION_H
#define AIRPORT_EXPANSION_H

#include "../Building.hpp"

constexpr int AIRPORT_EXPANSION_TICKS = 5;
constexpr LLint AIRPORT_EXPANSION_COST = 5'000LL;
constexpr LLint AIRPORT_EXPANSION_MONEY = 1'000LL;
constexpr LLint AIRPORT_EXPANSION_CO2 = 20'000LL;
constexpr LLint AIRPORT_EXPANSION_ENERGY = -30'000LL;
constexpr LLint AIRPORT_EXPANSION_WATER = -80'000LL;

class AirportExpansion : public Building {
    public:
        AirportExpansion();
        ~AirportExpansion() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
