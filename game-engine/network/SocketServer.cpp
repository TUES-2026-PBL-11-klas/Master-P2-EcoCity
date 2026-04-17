#include "SocketServer.hpp"
#include <iostream>
#include <cstring>

#include "../Logger.hpp"

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

SocketServer::SocketServer(int port)
    : port(port), running(true), serverFd(INVALID_SOCKET), clientFd(INVALID_SOCKET)
{
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);   // This has to be called before any socket functions on Windows, initialize Winsock library
    #endif

    listenerThread = std::thread(&SocketServer::listenLoop, this);
}

SocketServer::~SocketServer()
{
    stop();
}

void SocketServer::stop()
{
    running = false;    // Says to listenLoop to stop and exit
    if (clientFd != INVALID_SOCKET) closesocket(clientFd);
    if (serverFd != INVALID_SOCKET) closesocket(serverFd);
    if (listenerThread.joinable()) listenerThread.join();

    #ifdef _WIN32
        WSACleanup();
    #endif
}

void SocketServer::listenLoop()
{
    // Create socket, AF_INET == IPv4, SOCK_STREAM == TCP
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == INVALID_SOCKET) {
        // std::cerr << "Failed to create socket\n";
        LOG_ERROR("SocketServer", "socket_fail", "socket_creation_failed");
        return;
    }

    int opt = 1;
    // Allow quick reuse of the port after the server is stopped
    #ifdef _WIN32
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    #else
        setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif

    // Describe the address we want to bind to: IPv4, any local IP, specified port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        // std::cerr << "Bind failed\n";
        LOG_ERROR("SocketServer", "socket_fail", "socket_bind_failed");
        return;
    }

    listen(serverFd, 1); // Listen for incoming connections, backlog of 1 is enough since we only expect one client UI
    // std::cout << "Waiting for UI connection on port " << port << "...\n";
    LOG_INFO("SocketServer", "waiting_connection", "port=" + std::to_string(port));

    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    clientFd = accept(serverFd, (sockaddr*)&clientAddr, &clientLen);    // Wait for a client to connect
    if (clientFd == INVALID_SOCKET) return;

    // std::cout << "UI connected.\n";
    LOG_INFO("SocketServer", "ui_connected");

    while (running) {
        std::string data = receiveMessage();
        if (data.empty())
        {
            LOG_WARN("SocketServer", "ui_disconnected");
            break;  // UI disconnected
        }

        game_api::v1::UIAction action;
        if (action.ParseFromString(data)) {
            std::lock_guard<std::mutex> lock(actionMutex);  // lock while updating pendingAction, auto unlocks at the end of scope
            pendingAction = action;
        }
    }
}

// Length-prefixed framing: 4 bytes length, then payload
std::string SocketServer::receiveMessage()
{
    // Read 4-byte length header
    uint8_t header[4];
    int received = 0;
    while (received < 4)
    {
        #ifdef _WIN32
                int n = recv(clientFd, (char*)(header + received), 4 - received, 0);    // Read header, might not get all 4 bytes at once, so loop until we do
        #else
                int n = recv(clientFd, header + received, 4 - received, 0);
        #endif

        if (n <= 0) return "";
        received += n;
    }

    uint32_t length = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
    if (length == 0 || length > 1024 * 1024) return ""; // sanity check, something went wrong if length is 0 or larger than 1MB

    std::string data(length, '\0');
    received = 0;
    while (received < (int)length)
    {
        #ifdef _WIN32
                int n = recv(clientFd, &data[received], length - received, 0);
        #else
                int n = recv(clientFd, &data[received], length - received, 0);
        #endif

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
    std::lock_guard<std::mutex> lock(actionMutex);  // lock while accessing pendingAction
    auto action = pendingAction;                    // copy the current pending action
    pendingAction.reset();                          // clear after reading, so we don't prossess it again
    return action;
}

// Converts GameState proto obj to string and sends it to UI
void SocketServer::sendGameState(const game_api::v1::GameState& state)
{
    std::string serialized;
    if (state.SerializeToString(&serialized)) {
        sendBytes(serialized);
    }
}
