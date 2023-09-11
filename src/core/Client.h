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

enum class ClientState
{
    StartScreen,
    Game,
};

class Client
{
public:
    Client(Console& console, Net& net);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool runFrame();

    void shutdown();

    void showMenu();

    void hideMenu();

    void changeState(ClientState state);

private:
    Console& console;

    Net& net;

    std::unique_ptr<FileManager> fileManager;

    std::unique_ptr<EventQueue> eventQueue;

    std::unique_ptr<EventHandler> eventHandler;

    std::unique_ptr<Renderer> renderer;

    std::unique_ptr<Timer> timer;

    uint64_t lastTick;

    std::unique_ptr<Menu> menu;

    ClientState clientState;

    bool running;

    bool menuVisible;

    void handleEvents();

    bool consumeEvent(const Event& ev);

    void tryRunTicks();

    void draw();
};
