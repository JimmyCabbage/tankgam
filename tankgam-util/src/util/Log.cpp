#include "util/Log.h"

const char* getLogLevelString(LogLevel level)
{
    const char* levelName = "Unknown";
    switch (level)
    {
    case LogLevel::Debug:
        levelName = "Debug";
        break;
    case LogLevel::Info:
        levelName = "Info";
        break;
    case LogLevel::Warning:
        levelName = "Warning";
        break;
    case LogLevel::Error:
        levelName = "Error";
        break;
    }

    return levelName;
}
