#ifndef BUILDING_TYPE_H
#define BUILDING_TYPE_H

// If you change this enum, make sure to update the .proto file, add a new building type in buildings and update City constructor

enum BuildingType{
    BUILDING_UNSPECIFIED,
    POWER_PLANT,
    WATER_TREATMENT_PLANT,
    SOLAR_PANEL_FARM
};

#endif
