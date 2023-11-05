#pragma once

#include <vector>
#include <string>

#include <fmt/printf.h>
#include "SDL.h"

class Console
{
public:
    Console();
    ~Console();
    
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    
    //printf style log
    //note: this will probably lead to some exploits that'll kill me
    void logf(std::string_view format, ...);
    
    void log(std::string_view line);
    
private:
    std::vector<std::string> lines;
};
