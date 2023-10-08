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

    uint32_t lastAckedEntityManager;
    std::array<bool, EntityManager::NUM_ENTITY_MANAGERS> ackedEntityManagers;
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
    
    uint32_t currentEntityManager;
    std::array<uint32_t, EntityManager::NUM_ENTITY_MANAGERS> entityManagerSequences;
    std::array<std::unique_ptr<EntityManager>, EntityManager::NUM_ENTITY_MANAGERS> entityManagers;

    bool running;
    
    bool bleh = false;
    
//entity manager stuff
    EntityManager* getEntityManager(uint32_t sequence);
    
    EntityManager& insertEntityManager(uint32_t sequence);
    
    EntityId allocateGlobalEntity(Entity globalEntity);
    
    void freeGlobalEntity(EntityId netEntityId);
    
//main loop stuff
    void nextFrameSettings();

    void handlePackets();

    void handleUnconnectedPacket(NetBuf& buf, const NetAddr& fromAddr);

    void handleReliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& theClient);
    
    void handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& theClient);

    void handleEvents();

    void tryRunTicks();
    
    void sendPackets();
};
