#pragma once

#include <memory>
#include <vector>
#include <array>

#include "EntityManager.h"

class Console;
class FileManager;
class Timer;
class Net;
class NetBuf;
class NetChan;
struct NetAddr;
enum class NetMessageType : uint8_t;

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
    
    std::unique_ptr<EntityManager> entityManager;

    bool running;
    
    bool bleh = false;
    
    uint64_t lastTick;
    uint64_t currentTick;
    
    float rotationAmount = 0.0f;
    
    EntityId allocateGlobalEntity(Entity globalEntity);
    
    void freeGlobalEntity(EntityId netEntityId);
    
//main loop stuff
    void handlePackets();

    void handleUnconnectedPacket(NetBuf& buf, const NetAddr& fromAddr);

    void handleReliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& theClient);
    
    void handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& theClient);

    void handleEvents();

    void tryRunTicks();
    
    void sendPackets();
};
