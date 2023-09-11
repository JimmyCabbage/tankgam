#pragma once

#include <memory>

class Console;
class FileManager;
class Timer;
class Net;

class Server
{
public:
    Server(Console& console, Net& net);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool runFrame();

    void shutdown();

private:
    Console& console;

    Net& net;

    std::unique_ptr<FileManager> fileManager;

    std::unique_ptr<Timer> timer;

    uint64_t lastTick;

    bool running;

    void handleEvents();

    void tryRunTicks();
};
