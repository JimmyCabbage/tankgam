#pragma once

#include <util/Log.h>

class StdLog : public Log
{
public:
    StdLog();
    ~StdLog();
    
    void logf(std::string_view format, ...) override;
    
    void log(std::string_view line) override;
};
