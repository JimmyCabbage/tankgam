#pragma once

#include <vector>
#include <string>

#include <fmt/printf.h>
#include "SDL.h"

#include <util/Log.h>

class Console : public Log
{
public:
    explicit Console(bool printDebug);
    ~Console() override;
    
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    
    //printf style log
    //note: this will probably lead to some exploits that'll kill me
    void logf(LogLevel logLevel, std::string_view format, ...) override;

    //logs at LogLevel::Info by default
    void logf(std::string_view format, ...) override;

    //single line
    void log(LogLevel logLevel, std::string_view line) override;

    //logs at LogLevel::Info by default
    void log(std::string_view line) override;
    
private:
    bool printDebug;
    //std::vector<std::string> lines;
};
