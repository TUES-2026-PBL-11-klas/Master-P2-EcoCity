#ifndef CITY_H
#define CITY_H

#include "BuildingType.hpp"
#include <unordered_map>

class City{
    private:
        std::unordered_map<BuildingType, int> buildings;

    public:
        City();
        void addBuilding(BuildingType type);
        int getBuildingCount(BuildingType type) const;
};

#endif
