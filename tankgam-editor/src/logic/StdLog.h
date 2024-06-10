#pragma once

#include <util/Log.h>

class StdLog : public Log
{
public:
    StdLog();
    ~StdLog();
    
    void logf(LogLevel logLevel, std::string_view format, ...) override;

    void logf(std::string_view format, ...) override;
    
    void log(LogLevel logLevel, std::string_view line) override;

    void log(std::string_view line) override;
};
