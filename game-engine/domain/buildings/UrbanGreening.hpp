#ifndef URBAN_GREENING_H
#define URBAN_GREENING_H

#include "../Building.hpp"

class UrbanGreening : public Building {
    public:
        UrbanGreening();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
