#ifndef PETITION_H
#define PETITION_H

#include "ResourceEffect.hpp"
#include "Building.hpp"
#include <string>
#include <vector>

class Petition{
    private:
        int id;
        Building* building;

    public:
        Petition(int id, Building* building);
        int getId() const;
        Building* getBuilding() const;
        const std::vector<ResourceEffect> buildTick();
        ~Petition();

        Petition(const Petition&) = delete;
        Petition& operator=(const Petition&) = delete;
};

#endif
