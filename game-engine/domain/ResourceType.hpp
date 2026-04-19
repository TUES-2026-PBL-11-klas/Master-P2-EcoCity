#ifndef RESOURCE_TYPE_H
#define RESOURCE_TYPE_H

// Any changes to this enum must be reflected in ResourceManager.cpp

// Units used:
// - WATER is measured in kiloliters (1 unit = 1,000 L)
// - ENERGY is measured in megawatt-hours (1 unit = 1 MWh)
// - MONEY is measured in 100,000 GBP
// - CO2 is measured in 5-ton units
// - POPULATION is measured in people

enum ResourceType {
    RESOURCE_UNSPECIFIED,
    WATER,
    ENERGY,
    MONEY,
    POPULATION,
    CO2
};

#endif
