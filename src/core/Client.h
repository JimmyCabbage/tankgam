#pragma once

#include <memory>
#include <array>
#include <unordered_map>

#include "Event.h"
#include "EntityManager.h"

class Console;
class FileManager;
class EventHandler;
class Renderer;
class Timer;
class Menu;
struct Model;
class EventQueue;
class Net;
class NetChan;
class NetBuf;
struct NetAddr;
enum class NetMessageType : uint8_t;

enum class ClientState
{
    Disconnected,
    Connecting,
    Connected,
};

class Client
{
public:
    Client(Console& console, FileManager& fileManager, Net& net);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool runFrame();

    void shutdown();

    void showMenu();

    void hideMenu();

    //void changeState(ClientState state);

private:
    Console& console;

    FileManager& fileManager;

    ClientState clientState;

    Net& net;
    std::unique_ptr<NetChan> netChan;

    std::unique_ptr<EventQueue> eventQueue;

    std::unique_ptr<EventHandler> eventHandler;

    std::unique_ptr<Renderer> renderer;

    std::unique_ptr<Timer> timer;

    std::unique_ptr<Menu> menu;
    
    uint32_t lastAckedEntityManager;
    uint32_t currentEntityManager;
    std::array<uint32_t, EntityManager::NUM_ENTITY_MANAGERS> entityManagerSequences;
    std::array<bool, EntityManager::NUM_ENTITY_MANAGERS> ackedEntityManagers;
    std::array<std::unique_ptr<EntityManager>, EntityManager::NUM_ENTITY_MANAGERS> entityManagers;
    
    std::unordered_map<std::string, std::unique_ptr<Model>> models;

    bool running;

    bool menuVisible;
    
    uint64_t lastTick;
    
//entity manager stuff
    EntityManager* getEntityManager(uint32_t sequence);
    
    EntityManager& insertEntityManager(uint32_t sequence);

//basic commands
    void connectToServer(NetAddr serverAddr);

    void disconnect();

//main loop stuff
    void nextFrameSettings();

    void handlePackets();

    void handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr);

    void handleReliablePacket(NetBuf& buf, const NetMessageType& msgType);
    
    void handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType);

    void handleEvents();

    bool consumeEvent(const Event& ev);

    void tryRunTicks();

    void draw();
};
