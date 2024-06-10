#pragma once

#include <memory>
#include <vector>
#include <array>

#include "EntityManager.h"

class Log;
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
    Challenging,
    Connected,
    Spawned
};

struct ServerClient
{
    ServerClientState state;

    std::unique_ptr<NetChan> netChan;
    
    uint64_t lastRecievedTime;
    uint32_t clientSalt;
    uint32_t serverSalt;
    uint32_t combinedSalt;
};

class Server
{
public:
    Server(Log& log, FileManager& fileManager, Net& net);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool runFrame();

    void shutdown();

private:
    Log& log;

    FileManager& fileManager;

    Net& net;
    std::vector<ServerClient> clients;

    std::unique_ptr<Timer> timer;
    
    std::unique_ptr<EntityManager> entityManager;

    bool running;
    
    uint64_t lastTick;
    uint64_t currentTick;
    
    float rotationAmount = 0.0f;
    
    EntityId allocateGlobalEntity(Entity globalEntity);
    
    void freeGlobalEntity(EntityId netEntityId);
    
    void disconnectClient(ServerClient& client, bool forceDisconnect);
    
//main loop stuff
    void handlePackets();

    void handleUnconnectedPacket(NetBuf& buf, const NetAddr& fromAddr);

    void handleReliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& client);
    
    void handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& client);

    void handleEvents();

    void tryRunTicks();
    
    void sendPackets();
};
