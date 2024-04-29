#include "Console.h"

#include <cstdarg>

Console::Console(bool printDebug)
    : printDebug{ printDebug }
{

}

Console::~Console() = default;

static constexpr size_t CONSOLE_BUFFER_SIZE = 1024;

void Console::logf(LogLevel logLevel, std::string_view format, ...)
{
    va_list args;
    va_start(args, format);

    char buf[CONSOLE_BUFFER_SIZE];
    std::vsnprintf(buf, sizeof buf, format.data(), args);

    va_end(args);

    log(logLevel, buf);
}

void Console::logf(std::string_view format, ...)
{
    va_list args;
    va_start(args, format);

    char buf[CONSOLE_BUFFER_SIZE];
    std::vsnprintf(buf, sizeof buf, format.data(), args);

    va_end(args);

    log(LogLevel::Info, buf);
}

void Console::log(LogLevel logLevel, std::string_view line)
{
    int category = SDL_LOG_CATEGORY_APPLICATION;

    SDL_LogPriority priority = SDL_LOG_PRIORITY_VERBOSE;
    switch (logLevel)
    {
    case LogLevel::Verbose:
        priority = SDL_LOG_PRIORITY_VERBOSE;
        break;
    case LogLevel::Debug:
        if (printDebug)
        {
            priority = SDL_LOG_PRIORITY_DEBUG;
            break;
        }
        return;
    case LogLevel::Info:
        priority = SDL_LOG_PRIORITY_INFO;
        break;
    case LogLevel::Warning:
        priority = SDL_LOG_PRIORITY_WARN;
        break;
    case LogLevel::Error:
        priority = SDL_LOG_PRIORITY_ERROR;
        break;
    }

    SDL_LogMessage(category, priority, "%s", line.data());

    //lines.emplace_back(line.data());
}

void Console::log(std::string_view line)
{
    log(LogLevel::Info, line);
}
