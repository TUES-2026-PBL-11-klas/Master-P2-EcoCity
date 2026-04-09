#include "PublicTransportUpgrade.hpp"

namespace {
constexpr std::int64_t PUBLIC_TRANSPORT_COST = 150'000LL;
constexpr int PUBLIC_TRANSPORT_TICKS = 6;
constexpr std::int64_t PUBLIC_TRANSPORT_CO2 = -1'500LL;
}

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
