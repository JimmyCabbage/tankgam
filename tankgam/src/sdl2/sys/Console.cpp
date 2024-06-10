#include "Console.h"

#if __linux
    #include <unistd.h>
#endif

#include <cstdarg>

Console::Console(bool printDebug)
{
    SDL_LogSetOutputFunction([](void* userdata, int category, SDL_LogPriority priority, const char* msg)
        {
            Console* console = static_cast<Console*>(userdata);
            console->sdl2LogOutputFunction(category, priority, msg);
        }, this);

    if (printDebug)
    {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
    }
    else
    {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    }
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
    case LogLevel::Debug:
        priority = SDL_LOG_PRIORITY_DEBUG;
        break;
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

void Console::sdl2LogOutputFunction(int category, SDL_LogPriority priority, const char* msg)
{
    const int categoryPriority = SDL_LogGetPriority(category);
    if (priority < categoryPriority)
    {
        return;
    }

    const char* priorityStr = nullptr;
    switch (priority)
    {
    case SDL_LOG_PRIORITY_VERBOSE:
        priorityStr = "VERBOSE:";
        break;
    case SDL_LOG_PRIORITY_DEBUG:
        priorityStr = "DEBUG:";
        break;
    case SDL_LOG_PRIORITY_INFO:
        priorityStr = "INFO:";
        break;
    case SDL_LOG_PRIORITY_WARN:
        priorityStr = "WARN:";
        break;
    case SDL_LOG_PRIORITY_ERROR:
        priorityStr = "ERROR:";
        break;
    case SDL_LOG_PRIORITY_CRITICAL:
        priorityStr = "CRITICAL:";
        break;
    default:
        priorityStr = "UNKNOWN:";
        break;
    }

#if __linux
    //add colour if we're linux & probably a tty
    if (isatty(STDERR_FILENO))
    {
        const char* colour = nullptr;
        switch (priority)
        {
        case SDL_LOG_PRIORITY_VERBOSE:
        case SDL_LOG_PRIORITY_DEBUG:
            colour = "\033[1;34m";
            break;
        case SDL_LOG_PRIORITY_INFO:
            colour = "\033[1;32m";
            break;
        case SDL_LOG_PRIORITY_WARN:
            colour = "\033[1;33m";
            break;
        case SDL_LOG_PRIORITY_ERROR:
            priorityStr = "\033[1;31m";
            break;
        default:
            priorityStr = "";
            break;
        }

        fprintf(stderr, "%s%-10s%s\033[0m\n", colour, priorityStr, msg);
    }
    else
    {
        fprintf(stderr, "%-10s%s\n", priorityStr, msg);
    }
#else
    fprintf(stderr, "%s%s\n", priorityStr, msg);
#endif
}
