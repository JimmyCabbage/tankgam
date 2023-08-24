#pragma once

#include <memory>
#include <deque>
#include <optional>
#include <unordered_map>
#include <cstdint>
#include <array>

#include "Event.h"

class Console;
class FileManager;
class EventHandler;
class Renderer;
class Timer;
class Menu;
struct Model;
class EventQueue;

enum class GameState
{
    StartScreen,
    Level,
};

class Engine
{
public:
    Engine();
    ~Engine();
    
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    
    void loop();
    
    void shutdown();
    
    void showMenu();
    
    void hideMenu();
    
    void changeState(GameState state);
    
private:
    std::unique_ptr<Console> console;
    
    std::unique_ptr<FileManager> fileManager;
    
    std::unique_ptr<EventQueue> eventQueue;
    
    std::unique_ptr<EventHandler> eventHandler;
    
    std::unique_ptr<Renderer> renderer;
    
    std::unique_ptr<Timer> timer;
    
    uint64_t lastTick;
    
    std::unique_ptr<Menu> menu;
    
    GameState gameState;
    
    bool running;
    
    bool menuVisible;
    
    void handleEvents();

    bool consumeEvent(const Event& ev);
    
    void tryRunTicks();
    
    void draw();
};
