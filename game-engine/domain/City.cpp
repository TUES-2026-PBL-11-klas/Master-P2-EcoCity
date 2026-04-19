#include "City.hpp"

//Single Responsibility Principle - only tracks what types and the number of buildings built
City::City()
{
    buildings[BUILDING_UNSPECIFIED] = 0;
    buildings[POWER_PLANT] = 0;
    buildings[WATER_TREATMENT_PLANT] = 0;
    buildings[SOLAR_PANEL_FARM] = 0;
    buildings[SOLAR_PANEL_ROOFTOPS] = 0;
    buildings[PUBLIC_TRANSPORT_UPGRADE] = 0;
    buildings[WIND_TURBINE_FARM] = 0;
    buildings[HYDROELECTRIC_PLANT] = 0;
    buildings[URBAN_GREENING] = 0;
    buildings[WATER_SAVING_INFRASTRUCTURE] = 0;
    buildings[INDUSTRIAL_ZONE] = 0;
    buildings[AIRPORT_EXPANSION] = 0;
    buildings[ROAD_IMPROVEMENT] = 0;
}

void City::addBuilding(BuildingType type)
{
    buildings[type]++;
}

int City::getBuildingCount(BuildingType type) const
{
    return buildings.at(type);
}

const std::unordered_map<BuildingType, int>& City::getBuildings() const
{
    return buildings;
}
