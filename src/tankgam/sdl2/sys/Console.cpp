#include "Console.h"

#include <cstdarg>

Console::Console() = default;

Console::~Console() = default;

void Console::logf(std::string_view format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buf[512];
    std::vsnprintf(buf, sizeof buf, format.data(), args);
    
    va_end(args);
    
    SDL_Log("%s", buf);
    
    lines.emplace_back(buf);
}

void Console::log(std::string_view line)
{
    SDL_Log("%s", line.data());

    lines.emplace_back(line.data());
}
