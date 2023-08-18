#pragma once

#include <memory>
#include <deque>
#include <optional>
#include <unordered_map>
#include <cstdint>
#include <array>

#include "Event.h"
#include "Thing.h"
#include "Player.h"

class Console;
class FileManager;
class EventHandler;
class Renderer;
class RenderableManager;
class ThinkerManager;
class Timer;
class Menu;
class World;
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
    
    uint32_t registerThing(ThingType type, ThingFlags flags, phys::Box box);
    
    void unregisterThing(uint32_t id);
    
    Thing& getThing(uint32_t id);
    
    Console& getConsole();
    
    FileManager& getFileManager();
    
    EventQueue& getEventQueue();
    
    Renderer& getRenderer();
    
    RenderableManager& getRenderableManager();
    
    World& getWorld();
    
private:
    std::unique_ptr<Console> console;
    
    std::unique_ptr<FileManager> fileManager;
    
    std::unique_ptr<EventQueue> eventQueue;
    
    std::unique_ptr<EventHandler> eventHandler;
    
    std::unique_ptr<Renderer> renderer;
    
    std::unique_ptr<RenderableManager> renderableManager;
    
    std::unique_ptr<ThinkerManager> thinkerManager;
    
    std::unique_ptr<Timer> timer;
    
    uint64_t lastTick;
    
    std::unique_ptr<Menu> menu;
    
    std::unique_ptr<World> world;
    
    GameState gameState;
    
    bool running;
    
    bool menuVisible;
    
    std::unique_ptr<Model> model;
    std::unique_ptr<Model> jobby;
    
    void handleEvents();
    
    void tryRunTicks();
    
    void draw();
};
