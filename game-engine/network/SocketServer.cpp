#include "SocketServer.hpp"
#include <iostream>
#include <cstring>
#include <stdexcept>

#include "../observability/Logger.hpp"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

//Single Responsibility Principle - handles only the communication between the game_engine and the UI.

SocketServer::SocketServer(int port)
    : port(port), running(true), serverFd(INVALID_SOCKET), clientFd(INVALID_SOCKET)
{
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif

    listenerThread = std::thread(&SocketServer::listenLoop, this);

    // Block until listenLoop signals that the socket is ready (or failed).
    // If it failed, get_future().get() rethrows the stored exception so
    // the caller sees a std::runtime_error instead of a silent background failure.
    startupPromise_.get_future().get();
}

SocketServer::~SocketServer()
{
    stop();
}

void SocketServer::stop()
{
    running = false;
    if (clientFd != INVALID_SOCKET) closesocket(clientFd);
    if (serverFd != INVALID_SOCKET) closesocket(serverFd);
    if (listenerThread.joinable()) listenerThread.join();

    #ifdef _WIN32
        WSACleanup();
    #endif
}

void SocketServer::listenLoop()
{
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == INVALID_SOCKET) {
        LOG_ERROR("SocketServer", "socket_fail", "socket_creation_failed");
        // Propagate the failure to the constructor via the promise.
        startupPromise_.set_exception(
            std::make_exception_ptr(
                std::runtime_error("SocketServer: failed to create socket")));
        return;
    }

    int opt = 1;
    #ifdef _WIN32
        const char* optPtr = reinterpret_cast<const char*>(&opt);
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, optPtr, sizeof(opt));
    #else
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        LOG_ERROR("SocketServer", "socket_fail", "socket_bind_failed");
        startupPromise_.set_exception(
            std::make_exception_ptr(
                std::runtime_error("SocketServer: failed to bind on port " + std::to_string(port))));
        return;
    }

    listen(serverFd, 1); // Listen for incoming connections, backlog of 1 is enough since we only expect one client UI

    LOG_INFO("SocketServer", "waiting_connection", "port=" + std::to_string(port));

    // Signal the constructor that setup succeeded; it is now safe to proceed.
    startupPromise_.set_value();

    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    clientFd = accept(serverFd, (sockaddr*)&clientAddr, &clientLen);
    if (clientFd == INVALID_SOCKET) return;

    LOG_INFO("SocketServer", "ui_connected");

    while (running) {
        std::string data = receiveMessage();
        if (data.empty())
        {
            LOG_WARN("SocketServer", "ui_disconnected");
            break;
        }

        game_api::v1::UIAction action;
        if (action.ParseFromString(data)) {
            std::lock_guard<std::mutex> lock(actionMutex);
            pendingAction = action;
        }
    }
}

std::string SocketServer::receiveMessage()
{
    uint8_t header[4];
    int received = 0;
    while (received < 4)
    {
        #ifdef _WIN32
                int n = recv(clientFd, (char*)(header + received), 4 - received, 0);
        #else
                int n = recv(clientFd, header + received, 4 - received, 0);
        #endif

        if (n <= 0) return "";
        received += n;
    }

    uint32_t length = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
    if (length == 0 || length > 1024 * 1024) return "";

    std::string data(length, '\0');
    received = 0;
    while (received < (int)length)
    {
        int n = recv(clientFd, &data[received], length - received, 0);
        if (n <= 0) return "";
        received += n;
    }

    LOG_DEBUG("SocketServer", "received_message", "length=" + std::to_string(length));
    return data;
}

bool SocketServer::sendBytes(const std::string& data)
{
    if (clientFd == INVALID_SOCKET) return false;

    uint32_t length = data.size();
    uint8_t header[4] = {
        (uint8_t)(length >> 24),
        (uint8_t)(length >> 16),
        (uint8_t)(length >> 8),
        (uint8_t)(length)
    };

    #ifdef _WIN32
        if (send(clientFd, (const char*)header, 4, 0) == SOCKET_ERROR) return false;
        if (send(clientFd, data.c_str(), length, 0) == SOCKET_ERROR) return false;
    #else
        if (send(clientFd, header, 4, 0) == SOCKET_ERROR) return false;
        if (send(clientFd, data.c_str(), length, 0) == SOCKET_ERROR) return false;
    #endif

    LOG_DEBUG("SocketServer", "sent_message", "length=" + std::to_string(length));
    return true;
}

std::optional<game_api::v1::UIAction> SocketServer::pollAction()
{
    std::lock_guard<std::mutex> lock(actionMutex);
    auto action = pendingAction;
    pendingAction.reset();
    return action;
}

void SocketServer::sendGameState(const game_api::v1::GameState& state)
{
    std::string serialized;
    if (state.SerializeToString(&serialized)) {
        sendBytes(serialized);
    }
}

void SocketServer::sendGameOver(const game_api::v1::GameOver& gameOver)
{
    // The UI distinguishes a GameOver message from a regular GameState by reading
    // a 1-byte message-type prefix that we prepend to the payload:
    //   0x01 = GameState  (all existing traffic)
    //   0x02 = GameOver
    //
    // This keeps backward-compatibility: old UI clients that do not understand
    // the prefix will simply fail to parse the payload and disconnect cleanly.

    std::string serialized;
    if (!gameOver.SerializeToString(&serialized)) return;

    // Build the framed packet: [4-byte length][1-byte type=0x02][payload]
    uint32_t frameLen = 1 + static_cast<uint32_t>(serialized.size());
    uint8_t header[4] = {
        (uint8_t)(frameLen >> 24),
        (uint8_t)(frameLen >> 16),
        (uint8_t)(frameLen >> 8),
        (uint8_t)(frameLen)
    };

    #ifdef _WIN32
        send(clientFd, (const char*)header, 4, 0);
        uint8_t msgType = 0x02;
        send(clientFd, (const char*)&msgType, 1, 0);
        send(clientFd, serialized.c_str(), static_cast<int>(serialized.size()), 0);
    #else
        send(clientFd, header, 4, 0);
        uint8_t msgType = 0x02;
        send(clientFd, &msgType, 1, 0);
        send(clientFd, serialized.c_str(), serialized.size(), 0);
    #endif

    LOG_INFO("SocketServer", "game_over_sent");

    // Close the client socket immediately so the UI receives EOF right after.
    closesocket(clientFd);
    clientFd = INVALID_SOCKET;
}
