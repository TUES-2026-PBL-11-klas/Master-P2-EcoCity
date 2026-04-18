#ifndef IGAME_REPOSITORY_H
#define IGAME_REPOSITORY_H

#include <string>

#include "../domain/City.hpp"
#include "../services/PetitionManager.hpp"
#include "../services/ResourceManager.hpp"

class IGameRepository {
    public:
        virtual ~IGameRepository() = default;

        virtual void saveGame(
            const std::string& gameId,
            const ResourceManager& resourceManager,
            const PetitionManager& petitionManager,
            const City& city
        ) = 0;
};

#endif
