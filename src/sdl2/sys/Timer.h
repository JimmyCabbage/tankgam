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
   
    //get the amount of passed ticks since this was last called
    uint64_t getPassedTicks();
    
    static constexpr uint64_t TICK_RATE = 64;

private:
    bool enabled;
    
	//the time when the timer began
    uint64_t startTime;
	
	//the last tick that was recieved when getPassedTicks() was called
    uint64_t lastTick;
    
    bool paused;
    
    uint64_t pauseOffset;
};
