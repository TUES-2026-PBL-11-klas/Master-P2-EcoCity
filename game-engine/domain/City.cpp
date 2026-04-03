#include "BuildingType.hpp"
#include <unordered_map>
#include "City.hpp"

City::City()
{
    buildings[BUILDING_UNSPECIFIED] = 0;
    buildings[POWER_PLANT] = 0;
    buildings[WATER_TREATMENT_PLANT] = 0;
    buildings[SOLAR_PANEL_FARM] = 0;
}

void City::addBuilding(BuildingType type)
{
    buildings[type]++;
}

int City::getBuildingCount(BuildingType type) const
{
    return buildings.at(type);
}
