#include "sys/Timer.h"

#include <stdexcept>

#include "SDL.h"

Timer::Timer()
    : enabled{ false }, startTime{ 0 }, lastTick{ 0 },
      paused{ false }, pauseOffset{ 0 }
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
    startTime = SDL_GetTicks64();
    lastTick = 0;
    paused = false;
    pauseOffset = 0;
}

void Timer::stop()
{
    enabled = false;
    startTime = 0;
    lastTick = 0;
    paused = false;
    pauseOffset = 0;
}

void Timer::pause()
{
    paused = true;
}

void Timer::unpause()
{
    paused = false;
}

uint64_t Timer::getPassedTicks()
{
    if (enabled)
    {
        const uint64_t ticks = SDL_GetTicks64() - startTime - pauseOffset;
        if (paused)
        {
            pauseOffset += ticks - lastTick;
        }
        lastTick = ticks;
        
        return (ticks * Timer::TICK_RATE) / 1000;
    }
    else
    {
        return 0;
    }
}
