#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <atomic>
#include <future>
#include <mutex>
#include <thread>
#include <string>
#include <optional>

#include "api_types.pb.h"
#include "ISocketServer.hpp"

class SocketServer : public ISocketServer {
    private:
        int port;
        std::atomic<bool> running;
        std::thread listenerThread;

        std::mutex actionMutex;
        std::optional<game_api::v1::UIAction> pendingAction;

        // Promise/future pair used to signal startup success or failure back to
        // the constructor. The listener thread fulfils the promise once it either
        // starts listening successfully or hits a fatal socket error.
        std::promise<void> startupPromise_;

        // We all know what file descriptors are, right? that is what fd stands for
        int serverFd;
        int clientFd;

        void listenLoop();
        bool sendBytes(const std::string& data);
        std::string receiveMessage();

    public:
        explicit SocketServer(int port);
        ~SocketServer() override;

        std::optional<game_api::v1::UIAction> pollAction() override;
        void sendGameState(const game_api::v1::GameState& state) override;
        // Serialises and sends a GameOver message, then closes the client socket
        // so the UI receives EOF immediately after the final message.
        void sendGameOver(const game_api::v1::GameOver& gameOver) override;
        void stop() override;
};

#endif
