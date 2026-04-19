#include "PublicTransportUpgrade.hpp"

PublicTransportUpgrade::PublicTransportUpgrade()
    : Building(PUBLIC_TRANSPORT_UPGRADE, PUBLIC_TRANSPORT_COST, PUBLIC_TRANSPORT_TICKS)
{
    effects = Effects();
}

std::vector<ResourceEffect> PublicTransportUpgrade::Effects() const
{
    return {
        { CO2, PUBLIC_TRANSPORT_CO2 }
    };
}
