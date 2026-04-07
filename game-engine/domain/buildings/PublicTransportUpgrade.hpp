#ifndef PUBLIC_TRANSPORT_UPGRADE_H
#define PUBLIC_TRANSPORT_UPGRADE_H

#include "../Building.hpp"

class PublicTransportUpgrade : public Building {
    public:
        PublicTransportUpgrade();

    private:
        std::vector<ResourceEffect> Effects() const override;
};

#endif
