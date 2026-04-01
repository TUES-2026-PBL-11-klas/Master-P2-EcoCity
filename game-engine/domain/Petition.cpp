#include "ResourceEffect.hpp"
#include "Building.hpp"
#include <string>
#include <vector>
#include "Petition.hpp"

Petition::Petition(int id, Building* building)
{
    this->id = id;
    this->building = building;
}

Building* Petition::getBuilding() const
{
    return building;
}
int Petition::getId() const
{
    return id;
}

// This method will be called by the PetitionManager when ticking petitions.
// It will decrease the ticks to complete and return the effects if the petition is completed.
// Technically, it can be called by using getBuilding() and then buildTick(), but this is to simplify PetitionManager
const std::vector<ResourceEffect> Petition::buildTick()
{
    return building->buildTick();
}
