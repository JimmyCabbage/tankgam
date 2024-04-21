#pragma once

#include <vector>
#include <string>

#include <fmt/printf.h>
#include "SDL.h"

#include <util/Log.h>

class Console : public Log
{
public:
    Console();
    ~Console() override;
    
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    
    //printf style log
    //note: this will probably lead to some exploits that'll kill me
    void logf(std::string_view format, ...) override;
    
    void log(std::string_view line) override;
    
private:
    std::vector<std::string> lines;
};
