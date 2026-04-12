#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include <optional>

#include "../api_types.pb.h"

class SocketServer {
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
        ~SocketServer();

        // Called from socket thread: stores incoming UIAction
        // Called from game thread: reads and clears the latest action
        std::optional<game_api::v1::UIAction> pollAction();

        // Called from game thread: sends GameState to UI
        void sendGameState(const game_api::v1::GameState& state);

        void stop();
};

#endif
