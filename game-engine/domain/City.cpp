#include "BuildingType.hpp"
#include <unordered_map>

class City{
    private:
        std::unordered_map<BuildingType, int> buildings;

    public:
        City()
        {
            buildings[POWER_PLANT] = 0;
            buildings[WATER_TREATMENT_PLANT] = 0;
            buildings[SOLAR_PANEL_FARM] = 0;
        }
        void addBuilding(BuildingType type)
        {
            buildings[type]++;
        }
        int getBuildingCount(BuildingType type) const
        {
            return buildings.at(type);
        }
};
