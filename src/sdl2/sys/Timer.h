#pragma once

#include <cstdint>

class Engine;

class Timer
{
public:
    Timer();
    ~Timer();
    
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    
    void start();
    
    void stop();
    
    void pause();
    
    void unpause();
    
    uint64_t getTicks();
    
    static constexpr uint64_t TICK_RATE = 64;
    
private:
    bool enabled;
    
    uint64_t startTime;
    uint64_t lastTick;
    
    bool paused;
    
    uint64_t pauseOffset;
};
