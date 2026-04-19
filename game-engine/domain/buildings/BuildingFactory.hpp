#ifndef BUILDING_FACTORY_H
#define BUILDING_FACTORY_H

#include "../Building.hpp"

#include <stdexcept>

#include "AirportExpansion.hpp"
#include "HydroelectricPlant.hpp"
#include "IndustrialZone.hpp"
#include "PowerPlant.hpp"
#include "PublicTransportUpgrade.hpp"
#include "RoadImprovement.hpp"
#include "RooftopSolar.hpp"
#include "SolarPanelFarm.hpp"
#include "UrbanGreening.hpp"
#include "WaterSavingInfrastructure.hpp"
#include "WaterTreatmentPlant.hpp"
#include "WindTurbineFarm.hpp"

Building* createBuilding(BuildingType type);

#endif
