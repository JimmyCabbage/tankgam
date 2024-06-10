#pragma once

#include <memory>
#include <stack>

#include "Event.h"
#include "EntityManager.h"

class Log;
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
class IClientState;

class Client
{
public:
    friend class IClientState;

    Client(Log& log, FileManager& fileManager, Net& net);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool runFrame();

    void shutdown();

    void pushState(std::shared_ptr<IClientState> clientState);
    void popState();

private:
    Log& log;

    FileManager& fileManager;

    Net& net;

    std::unique_ptr<EventQueue> eventQueue;

    std::unique_ptr<EventHandler> eventHandler;

    std::unique_ptr<Renderer> renderer;

    bool running;

    uint64_t lastTick;
    uint64_t currentTick;

    std::stack<std::shared_ptr<IClientState>> stateStack;

//main loop stuff
    void handleEvents();

    bool consumeEvent(const Event& ev);

    void draw();
};
