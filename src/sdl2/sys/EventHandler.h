#pragma once

#include <vector>
#include <queue>

#include "SDL.h"

#include "Event.h"

class Engine;
class Renderer;

class EventHandler
{
public:
    explicit EventHandler(Engine& engine);
    ~EventHandler();
    
    EventHandler(EventHandler&) = delete;
    EventHandler& operator=(EventHandler&) = delete;
    
    void refreshEvents();

private:
    Engine& engine;
    
    Renderer& renderer;
    
    void convertEvent(SDL_Event& ev);
};
