#include "sys/Timer.h"

#include <stdexcept>

#include "SDL.h"

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
    startTime = SDL_GetTicks64();
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
        const uint64_t ticks = SDL_GetTicks64() - startTime;

        return (ticks * Timer::TICK_RATE) / 1000;
    }
    else
    {
        return 0;
    }
}
