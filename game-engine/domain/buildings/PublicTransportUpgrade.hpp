#ifndef PUBLIC_TRANSPORT_UPGRADE_H
#define PUBLIC_TRANSPORT_UPGRADE_H

#include "../Building.hpp"

constexpr LLint PUBLIC_TRANSPORT_COST = 150'000LL;
constexpr int PUBLIC_TRANSPORT_TICKS = 6;
constexpr LLint PUBLIC_TRANSPORT_CO2 = -1'500LL;

class PublicTransportUpgrade : public Building {
    public:
        PublicTransportUpgrade();
        ~PublicTransportUpgrade() override = default;

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
