#include "Engine.h"

#include <stdexcept>
#include <numeric>

#include <fmt/format.h>

#include "sys/Console.h"
#include "sys/File.h"
#include "sys/EventHandler.h"
#include "sys/Renderer.h"
#include "sys/Timer.h"
#include "Menu.h"
#include "Event.h"

Engine::Engine()
{
    console = std::make_unique<Console>();
    
    try
    {
        console->log("Init File Subsystem...");
        fileManager = std::make_unique<FileManager>(*console);
        fileManager->loadAssetsFile("dev.assets");
        fileManager->loadAssetsFile("tank.assets");
        
        console->log("Init Event Subsystem...");
        eventQueue = std::make_unique<EventQueue>();
        eventHandler = std::make_unique<EventHandler>(*eventQueue);
        
        console->log("Init Renderer Subsystem...");
        renderer = std::make_unique<Renderer>(*console, *fileManager, "src");

        console->log("Init Timer Subsystem...");
        timer = std::make_unique<Timer>();
        
        lastTick = 0;
        
        console->log("Init Menu Subsystem...");
        menu = std::make_unique<Menu>(*renderer);
        
        const auto mainListCallback = [this](size_t choice) -> void
        {
            switch (choice)
            {
            case 0:
                changeState(GameState::Level);
                break;
            case 1:
                shutdown();
                break;
            }
        };
        MenuList mainList{ mainListCallback };
        mainList.addChoice("Start Game");
        mainList.addChoice("End Game");
        menu->addList(std::move(mainList));
    }
    catch (const std::exception& e)
    {
        console->log(fmt::format("Init Error:\n{}", e.what()));
        throw e;
    }
    
    gameState = GameState::StartScreen;
    
    running = true;
    
    menuVisible = true;
    
    //playerThing = std::make_unique<Player>(*this);

    console->log("Initialized Engine!");
}

Engine::~Engine()
{
    console->log("Quitting Engine...");
}

void Engine::loop()
{
    try
    {
        while (running)
        {
            handleEvents();
            
            tryRunTicks();
            
            draw();
        }
    }
    catch (const std::exception& e)
    {
        console->log(fmt::format("Runtime Error:\n{}", e.what()));
        throw e;
    }
}

void Engine::shutdown()
{
    running = false;
}

void Engine::showMenu()
{
    menuVisible = true;
    
    timer->pause();
    console->log("Showing Menu");
}

void Engine::hideMenu()
{
    menuVisible = false;
    
    timer->unpause();
    console->log("Hiding Menu");
}

void Engine::changeState(GameState state)
{
    if (gameState == state)
    {
        return;
    }

    switch (state)
    {
    case GameState::StartScreen:
        timer->stop();
        console->log("Changing state to StartScreen");
        gameState = state;
        break;
    case GameState::Level:
        timer->start();
        console->log("Changing state to Level");
        menuVisible = false;
        gameState = state;
        lastTick = timer->getTicks();
        break;
    default:
        throw std::runtime_error{ "Tried to changeState to an unknown state" };
    }
}

void Engine::handleEvents()
{
    eventHandler->refreshEvents();
    
    Event ev{};
    while (eventQueue->popEvent(ev))
    {
        if (consumeEvent(ev))
        {
            continue;
        }

        if (menuVisible && menu->consumeEvent(ev))
        {
            continue;
        }
        
        if (renderer->consumeEvent(ev))
        {
            continue;
        }
    }
}

bool Engine::consumeEvent(const Event& ev)
{
    switch (ev.type)
    {
    case EventType::Quit:
        shutdown();
        break;
    }
}

void Engine::tryRunTicks()
{
    if (menuVisible)
    {
        return;
    }
    
    if (gameState == GameState::Level)
    {
        const uint64_t currTicks = timer->getTicks();
        const uint64_t ticks = currTicks - lastTick;
        lastTick = currTicks;
        
        if (ticks == 0)
        {
            return;
        }
        
        for (uint64_t i = 0; i < ticks; i++)
        {
            //for (auto& [id, thing] : things)
            {
            //    thing->think();
            }
        }
    }
}

void Engine::draw()
{
    renderer->beginDraw();
    
    renderer->drawText("HELLO WORLD ARE YOU THERE", glm::vec2{ 0.0f, 0.0f }, 100.0f);
    
    if (gameState == GameState::Level)
    {
        //if (playerId.has_value())
        {
            //things[playerId]
        }
        
        //for (auto& [id, thing] : things)
        {
        //    thing->draw();
        }
    }
    
    if (menuVisible)
    {
        menu->draw();
    }
    
    const float rotation = -static_cast<float>(lastTick % 360);

    renderer->endDraw();
}
