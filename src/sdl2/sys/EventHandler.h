#pragma once

#include <vector>
#include <queue>

#include "SDL.h"

#include "Event.h"

class EventQueue;

class EventHandler
{
public:
    explicit EventHandler(EventQueue& eventQueue);
    ~EventHandler();
    
    EventHandler(EventHandler&) = delete;
    EventHandler& operator=(EventHandler&) = delete;
    
    void refreshEvents();

private:
    EventQueue& eventQueue;

    void convertEvent(SDL_Event& ev);
};
