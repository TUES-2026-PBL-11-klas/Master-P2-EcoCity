#include "BuildingFactory.hpp"

Building* createBuilding(BuildingType type)
{
    switch (type) {
        case POWER_PLANT:
            return new PowerPlant();
        case WATER_TREATMENT_PLANT:
            return new WaterTreatmentPlant();
        case SOLAR_PANEL_FARM:
            return new SolarPanelFarm();
        case SOLAR_PANEL_ROOFTOPS:
            return new RooftopSolar();
        case PUBLIC_TRANSPORT_UPGRADE:
            return new PublicTransportUpgrade();
        case WIND_TURBINE_FARM:
            return new WindTurbineFarm();
        case HYDROELECTRIC_PLANT:
            return new HydroelectricPlant();
        case URBAN_GREENING:
            return new UrbanGreening();
        case WATER_SAVING_INFRASTRUCTURE:
            return new WaterSavingInfrastructure();
        case INDUSTRIAL_ZONE:
            return new IndustrialZone();
        case AIRPORT_EXPANSION:
            return new AirportExpansion();
        case ROAD_IMPROVEMENT:
            return new RoadImprovement();
        case BUILDING_UNSPECIFIED:
        default:
            throw std::invalid_argument("Unsupported building type");
    }
}
