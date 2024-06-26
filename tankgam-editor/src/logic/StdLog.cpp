#include "StdLog.h"

#include <cstdarg>
#include <cstdio>

StdLog::StdLog() = default;

StdLog::~StdLog() = default;

void StdLog::logf(LogLevel logLevel, std::string_view format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buf[512];
    std::vsnprintf(buf, sizeof buf, format.data(), args);
    
    va_end(args);

    log(logLevel, buf);
}

void StdLog::logf(std::string_view format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buf[512];
    std::vsnprintf(buf, sizeof buf, format.data(), args);
    
    va_end(args);

    log(LogLevel::Info, buf);
}

void StdLog::log(LogLevel logLevel, std::string_view line)
{
    const char* levelName = logLevelToString(logLevel);
    std::printf("%s: %s\n", levelName, line.data());
}

void StdLog::log(std::string_view line)
{
    log(LogLevel::Info, line);
}
