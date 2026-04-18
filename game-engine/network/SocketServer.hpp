#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include <optional>

#include "../api_types.pb.h"
#include "ISocketServer.hpp"

class SocketServer : public ISocketServer {
    private:
        int port;
        std::atomic<bool> running;
        std::thread listenerThread;

        std::mutex actionMutex;
        std::optional<game_api::v1::UIAction> pendingAction;

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
        void stop() override;
};

#endif
