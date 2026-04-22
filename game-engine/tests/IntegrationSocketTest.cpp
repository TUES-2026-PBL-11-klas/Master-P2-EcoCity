// Integration tests for SocketServer to proto framing.
// Spin up a real SocketServer on a localhost port, connect to it as a plain TCP client within the same process, and verify the send/receive pipeline.
// No external services required — everything runs on 127.0.0.1.

#include <gtest/gtest.h>

#include "../network/SocketServer.hpp"
#include "api_types.pb.h"
#include "../domain/ResourceType.hpp"
#include "../domain/BuildingType.hpp"

#include <chrono>
#include <cstring>
#include <string>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define closesocket    close
#endif

namespace {

// Minimal TCP client helpers

// Connect to 127.0.0.1:port, retrying for up to ~1 second while the
// SocketServer's listener thread starts up.
int connectToServer(int port)
{
    int fd = INVALID_SOCKET;
    for (int attempt = 0; attempt < 20; ++attempt) {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == INVALID_SOCKET) continue;

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(static_cast<uint16_t>(port));
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
            return fd; // success
        }

        closesocket(fd);
        fd = INVALID_SOCKET;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return INVALID_SOCKET;
}

// Send a 4-byte big-endian length header followed by payload bytes.
bool sendFramed(int fd, const std::string& payload)
{
    uint32_t len = static_cast<uint32_t>(payload.size());
    uint8_t header[4] = {
        static_cast<uint8_t>(len >> 24),
        static_cast<uint8_t>(len >> 16),
        static_cast<uint8_t>(len >>  8),
        static_cast<uint8_t>(len)
    };

    if (send(fd, reinterpret_cast<const char*>(header), 4, 0) != 4) return false;
    if (send(fd, payload.c_str(), len, 0) != static_cast<int>(len)) return false;
    return true;
}

// Receive exactly n bytes into buf (blocking, reassembles partial reads).
bool recvAll(int fd, char* buf, int n)
{
    int received = 0;
    while (received < n) {
        int r = recv(fd, buf + received, n - received, 0);
        if (r <= 0) return false;
        received += r;
    }
    return true;
}

// Receive one length-prefixed frame and return the payload.
std::string recvFramed(int fd)
{
    uint8_t header[4];
    if (!recvAll(fd, reinterpret_cast<char*>(header), 4)) return {};

    uint32_t len = (static_cast<uint32_t>(header[0]) << 24) |
                   (static_cast<uint32_t>(header[1]) << 16) |
                   (static_cast<uint32_t>(header[2]) <<  8) |
                    static_cast<uint32_t>(header[3]);

    if (len == 0 || len > 1024 * 1024) return {};

    std::string payload(len, '\0');
    if (!recvAll(fd, &payload[0], static_cast<int>(len))) return {};
    return payload;
}

// Poll pollAction() up to ~500 ms to give the listener thread time to deliver.
std::optional<game_api::v1::UIAction> pollWithTimeout(SocketServer& server)
{
    for (int i = 0; i < 50; ++i) {
        auto action = server.pollAction();
        if (action.has_value()) return action;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return std::nullopt;
}

// Each test uses a different port to avoid TIME_WAIT conflicts.
constexpr int BASE_PORT = 19100;

} // namespace

// Test 1: UI to backend (UIAction with petition acceptance)
TEST(IntegrationSocketTest, UIActionAcceptPetitionDeliveredToServer) {
    SocketServer server(BASE_PORT);

    int clientFd = connectToServer(BASE_PORT);
    ASSERT_NE(clientFd, INVALID_SOCKET) << "Could not connect to SocketServer";

    // Build a UIAction: accepted petition response
    game_api::v1::UIAction action;
    action.mutable_petition_response()->set_responded(true);
    action.mutable_petition_response()->set_accepted(true);

    std::string serialized;
    ASSERT_TRUE(action.SerializeToString(&serialized));
    ASSERT_TRUE(sendFramed(clientFd, serialized));

    // Server should deliver the action via pollAction()
    auto received = pollWithTimeout(server);

    closesocket(clientFd);
    server.stop();

    ASSERT_TRUE(received.has_value()) << "Server did not receive the UIAction within timeout";
    ASSERT_TRUE(received->has_petition_response());
    EXPECT_TRUE(received->petition_response().responded());
    EXPECT_TRUE(received->petition_response().accepted());
}

// Test 2: backend to UI (GameState serialization + framing)
TEST(IntegrationSocketTest, GameStateSentByServerCanBeDeserializedByClient) {
    SocketServer server(BASE_PORT + 1);

    int clientFd = connectToServer(BASE_PORT + 1);
    ASSERT_NE(clientFd, INVALID_SOCKET) << "Could not connect to SocketServer";

    // Give the listener thread a moment to accept the connection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Build a GameState with a few resources and a building count
    game_api::v1::GameState state;
    (*state.mutable_resources())[static_cast<int>(WATER)]      = 500'000;
    (*state.mutable_resources())[static_cast<int>(ENERGY)]     = 1'000'000;
    (*state.mutable_resources())[static_cast<int>(MONEY)]      = 250'000;
    (*state.mutable_resources())[static_cast<int>(POPULATION)] = 1'000'000;
    (*state.mutable_resources())[static_cast<int>(CO2)]        = 1'000'000;
    (*state.mutable_building_counts())[static_cast<int>(POWER_PLANT)] = 3;

    server.sendGameState(state);

    // Client reads the framed response
    std::string payload = recvFramed(clientFd);

    closesocket(clientFd);
    server.stop();

    ASSERT_FALSE(payload.empty()) << "Client received no data from server";

    game_api::v1::GameState decoded;
    ASSERT_TRUE(decoded.ParseFromString(payload)) << "Failed to parse GameState proto";

    EXPECT_EQ(decoded.resources().at(static_cast<int>(WATER)),      500'000);
    EXPECT_EQ(decoded.resources().at(static_cast<int>(ENERGY)),   1'000'000);
    EXPECT_EQ(decoded.resources().at(static_cast<int>(MONEY)),      250'000);
    EXPECT_EQ(decoded.resources().at(static_cast<int>(POPULATION)), 1'000'000);
    EXPECT_EQ(decoded.resources().at(static_cast<int>(CO2)),        1'000'000);
    EXPECT_EQ(decoded.building_counts().at(static_cast<int>(POWER_PLANT)), 3);
}

// Test 3: UI to backend (save_game flag)
TEST(IntegrationSocketTest, SaveGameFlagDeliveredToServer) {
    SocketServer server(BASE_PORT + 2);

    int clientFd = connectToServer(BASE_PORT + 2);
    ASSERT_NE(clientFd, INVALID_SOCKET) << "Could not connect to SocketServer";

    game_api::v1::UIAction action;
    action.set_save_game(true);

    std::string serialized;
    ASSERT_TRUE(action.SerializeToString(&serialized));
    ASSERT_TRUE(sendFramed(clientFd, serialized));

    auto received = pollWithTimeout(server);

    closesocket(clientFd);
    server.stop();

    ASSERT_TRUE(received.has_value());
    EXPECT_TRUE(received->save_game());
}

// Test 4: no action aka pollAction returns empty
TEST(IntegrationSocketTest, NoMessageSentPollActionReturnsEmpty) {
    SocketServer server(BASE_PORT + 3);

    int clientFd = connectToServer(BASE_PORT + 3);
    ASSERT_NE(clientFd, INVALID_SOCKET);

    // Don't send anything — poll immediately
    auto result = server.pollAction();

    closesocket(clientFd);
    server.stop();

    EXPECT_FALSE(result.has_value());
}
