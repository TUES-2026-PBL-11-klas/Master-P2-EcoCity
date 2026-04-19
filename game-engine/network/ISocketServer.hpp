#ifndef ISOCKET_SERVER_H
#define ISOCKET_SERVER_H

#include <optional>

#include "api_types.pb.h"

class ISocketServer {
    public:
        virtual ~ISocketServer() = default;
        virtual std::optional<game_api::v1::UIAction> pollAction() = 0;
        virtual void sendGameState(const game_api::v1::GameState& state) = 0;
        // Sends a GameOver message to the UI and then closes the connection.
        virtual void sendGameOver(const game_api::v1::GameOver& gameOver) = 0;
        virtual void stop() = 0;
};

#endif
