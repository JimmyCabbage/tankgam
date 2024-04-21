#include "StdLog.h"

#include <cstdarg>
#include <cstdio>

StdLog::StdLog() = default;

StdLog::~StdLog() = default;

void StdLog::logf(std::string_view format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buf[512];
    std::vsnprintf(buf, sizeof buf, format.data(), args);
    std::printf("%s\n", buf);
    
    va_end(args);
}

void StdLog::log(std::string_view line)
{
    std::printf("%s\n", line.data());
}
