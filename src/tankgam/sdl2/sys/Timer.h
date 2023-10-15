#pragma once

#include <cstdint>

class Timer
{
public:
    Timer();
    ~Timer();
    
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    
    void start();
    
    void stop();

    void setTickOffset(uint64_t tickOffset);

    //get the ticks since time started
    uint64_t getTotalTicks() const;
    
    static constexpr uint64_t TICK_RATE = 64;

private:
    bool enabled;

    //the time when the timer began
    uint64_t startTime;
};
