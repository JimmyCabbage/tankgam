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
    template <typename... Args>
    void logf(std::string_view format, Args... args)
    {
        SDL_Log(format.data(), args...);
        
        lines.push_back(fmt::sprintf(format.data(), args...));
    }
    
    void log(std::string_view line);
    
private:
    std::vector<std::string> lines;
};
