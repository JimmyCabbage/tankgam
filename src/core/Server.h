#pragma once

#include <memory>
#include <vector>

class Console;
class FileManager;
class Timer;
class Net;
class NetBuf;
class NetChan;
struct NetAddr;

enum class ServerClientState
{
    Free,
    Connected,
    Spawned
};

struct ServerClient
{
    ServerClientState state;

    std::unique_ptr<NetChan> netChan;
};

class Server
{
public:
    Server(Console& console, FileManager& fileManager, Net& net);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool runFrame();

    void shutdown();

private:
    Console& console;

    FileManager& fileManager;

    Net& net;
    std::vector<ServerClient> clients;

    std::unique_ptr<Timer> timer;

    bool running;

    void handlePackets();

    void handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr);

    void handleEvents();

    void tryRunTicks();
};
