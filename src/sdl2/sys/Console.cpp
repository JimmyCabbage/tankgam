#include "Console.h"

Console::Console() = default;

Console::~Console() = default;

void Console::log(std::string_view line)
{
    SDL_Log("%s", line.data());

    lines.push_back(line.data());
}
