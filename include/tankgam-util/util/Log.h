#pragma once

#include <string_view>

//a virtual pure class used to implement generic logging
class Log
{
public:
    Log() = default;
    virtual ~Log() = 0;
    
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    
    //printf style log
    //note: this will probably lead to some exploits that'll kill me
    virtual void logf(std::string_view format, ...) = 0;
    
    virtual void log(std::string_view line) = 0;
};

inline Log::~Log() {}
