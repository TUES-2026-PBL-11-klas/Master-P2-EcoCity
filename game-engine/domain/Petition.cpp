#include "ResourceEffect.hpp"
#include "Building.hpp"
#include <string>
#include <vector>

class Petition{
    private:
        int id;
        Building* building;

    public:
        Petition(int id, Building* building)
        {
            this->id = id;
            this->building = building;
        }
        int getCost() const
        {
            return building->getBuildCost();
        }
        int getTicksToComplete() const
        {
            return building->getTicksToComplete();
        }
        const std::vector<ResourceEffect>& getEffects() const
        {
            return building->getEffects();
        }
        int getId() const{
            return id;
        }
};