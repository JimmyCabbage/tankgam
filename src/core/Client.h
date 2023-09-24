#pragma once

#include <memory>

#include "Event.h"

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

    bool running;

    bool menuVisible;

//basic commands
    void connectToServer(NetAddr serverAddr);

    void disconnect();

//main loop stuff
    void handlePackets();

    void handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr);

    void handleEvents();

    bool consumeEvent(const Event& ev);

    void tryRunTicks();

    void draw();
};
