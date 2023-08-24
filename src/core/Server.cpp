#include "Server.h"

#include "sys/Console.h"
#include "sys/File.h"
#include "sys/Timer.h"

Server::Server(Console& console)
    : console{ console }
{
    try
    {
        console.log("Server: Init File Subsystem...");
        fileManager = std::make_unique<FileManager>(console);

        console.log("Server: Init Timer Subsystem...");
        timer = std::make_unique<Timer>();

        lastTick = 0;
    }
    catch (const std::exception& e)
    {
        console.log(fmt::format("Server: Init Error:\n{}", e.what()));
        throw e;
    }

    running = true;

    console.log("Server: Initialized");
}

Server::~Server()
{
    console.log("Server: Quitting");
}

bool Server::runFrame()
{
    try
    {
        handleEvents();

        tryRunTicks();
    }
    catch (const std::exception& e)
    {
        console.log(fmt::format("Server: Runtime Error:\n{}", e.what()));
        throw e;
    }

    return running;
}

void Server::shutdown()
{
    running = false;
}

void Server::handleEvents()
{

}

void Server::tryRunTicks()
{

}
