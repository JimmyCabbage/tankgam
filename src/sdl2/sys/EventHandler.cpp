#include "sys/EventHandler.h"

#include <stdexcept>
#include <array>

#include "SDL.h"

#include "sys/Renderer.h"
#include "Engine.h"

EventHandler::EventHandler(Engine& engine)
    : engine{ engine }, renderer{ engine.getRenderer() }
{
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
    {
        throw std::runtime_error{ "Failed to initialize SDL2 event subsystem" };
    }
}

EventHandler::~EventHandler()
{
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void EventHandler::refreshEvents()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_QUIT:
            engine.shutdown();
            break;
        }
        
        convertEvent(ev);
    }
}

KeyPressType convertKeycode(SDL_Keycode code)
{
    switch (code)
    {
    case SDLK_LEFT:      return KeyPressType::LeftArrow;
    case SDLK_RIGHT:     return KeyPressType::RightArrow;
    case SDLK_UP:        return KeyPressType::UpArrow;
    case SDLK_DOWN:      return KeyPressType::DownArrow;
    case SDLK_SPACE:     return KeyPressType::Space;
    case SDLK_BACKSPACE: return KeyPressType::Backspace;
    case SDLK_DELETE:    return KeyPressType::Delete;
    case SDLK_TAB:       return KeyPressType::Tab;
    case SDLK_ESCAPE:    return KeyPressType::Escape;
    case SDLK_RETURN:    return KeyPressType::Return;
    case SDLK_PERIOD:    return KeyPressType::Period;
    }
    
    if (code >= SDLK_0 && code <= SDLK_9)
    {
        return static_cast<KeyPressType>(code);
    }
    
    if (code >= SDLK_a && code <= SDLK_z)
    {
        return static_cast<KeyPressType>(code);
    }
    
    return KeyPressType::None;
}

void EventHandler::convertEvent(SDL_Event& ev)
{
    EventQueue& eventQueue = engine.getEventQueue();
    
    Event newEv{};
    switch (ev.type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        if (ev.type == SDL_KEYDOWN)
        {
            newEv.type = EventType::KeyDown;
        }
        else
        {
            newEv.type = EventType::KeyUp;
        }
        
        newEv.data1 = static_cast<uint32_t>(convertKeycode(ev.key.keysym.sym));
        if (newEv.data1 != static_cast<uint32_t>(KeyPressType::None))
        {
            eventQueue.pushEvent(newEv);
        }
        break;
    case SDL_WINDOWEVENT:
        switch (ev.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            newEv.type = EventType::WindowResize;
            newEv.data1 = ev.window.data1;
            newEv.data2 = ev.window.data2;
            
            eventQueue.pushEvent(newEv);
            break;
        }
    }
}
