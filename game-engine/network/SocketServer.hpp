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
        std::atomic<bool> running;      // Flag both threads can check, atomic is used because plain bool is not thread-safe
        std::thread listenerThread;     // thread that waits and gets info from UI, runs listenLoop() in parallel to the main game loop

        std::mutex actionMutex;         // Protects pendingAction, since it's shared between threads
        std::optional<game_api::v1::UIAction> pendingAction;    // Socket thread writes here, game thread reads it

        // We all know what file decriptors are, right? that is what fd stands for
        int serverFd;
        int clientFd;

        void listenLoop();
        bool sendBytes(const std::string& data);
        std::string receiveMessage();

    public:
        explicit SocketServer(int port);
        ~SocketServer() override;

        // Called from socket thread - stores coming UIActions
        // Called from game thread - reads and clears the latest action
        std::optional<game_api::v1::UIAction> pollAction() override;

        // Called from game thread: sends GameState to UI
        void sendGameState(const game_api::v1::GameState& state) override;

        void stop() override;    // closes socket and stops the listener thread
};

#endif
