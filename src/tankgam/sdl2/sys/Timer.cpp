#include "sys/Timer.h"

#include <stdexcept>

#include "SDL.h"

//since the 64 bit api of the tick system in sdl2 is only available past 2.0.18
//add this inbetween function to get it working on old sdl2 versions
static uint64_t GetTicks()
{
#if SDL_VERSION_ATLEAST(2, 0, 18)
    return SDL_GetTicks64();
#else
    return static_cast<uint64_t>(SDL_GetTicks());
#endif
}

Timer::Timer()
    : enabled{ false }, startTime{ 0 }
{
    if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
    {
        throw std::runtime_error{ "Failed to init SDL2 timer subsystem" };
    }
}

Timer::~Timer()
{
    SDL_QuitSubSystem(SDL_INIT_TIMER);
}

void Timer::start()
{
    enabled = true;
    startTime = GetTicks();
}

void Timer::stop()
{
    enabled = false;
    startTime = 0;
}

void Timer::setTickOffset(uint64_t tickOffset)
{
    startTime = startTime + ((tickOffset / 1000) * Timer::TICK_RATE);
}

//get the ticks since time started
uint64_t Timer::getTotalTicks() const
{
    if (enabled)
    {
        const uint64_t ticks = GetTicks() - startTime;

        return (ticks * Timer::TICK_RATE) / 1000;
    }
    else
    {
        return 0;
    }
}
