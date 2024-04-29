#pragma once

#include <string_view>

enum class LogLevel
{
    Verbose     = 1 << 0,
    Debug       = 1 << 1,
    Info        = 1 << 2,
    Warning     = 1 << 3,
    Error       = 1 << 4
};

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
    virtual void logf(LogLevel logLevel, std::string_view format, ...) = 0;

    //same as above but with logLevel = LogLevel::Info
    virtual void logf(std::string_view format, ...) = 0;

    //just print out line
    virtual void log(LogLevel logLevel, std::string_view line) = 0;

    //same as above but with logLevel = LogLevel::Info
    virtual void log(std::string_view line) = 0;
};

inline Log::~Log() = default;
