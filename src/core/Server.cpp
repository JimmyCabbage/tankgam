#include "Server.h"

#include "sys/Console.h"
#include "sys/File.h"
#include "sys/Timer.h"
#include "sys/Net.h"
#include "sys/NetChan.h"

Server::Server(Console& console, Net& net)
    : console{ console }, net{ net }
{
    try
    {
        netChan = std::make_unique<NetChan>(net, NetSrc::Client);

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
        handlePackets();

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

void Server::handlePackets()
{
    NetBuf buf{};
    NetAddr fromAddr{};
    while (net.getPacket(NetSrc::Server, buf, fromAddr))
    {
        //read the first byte of the msg
        //if it's -1 then it's an unconnected message
        if (*reinterpret_cast<const uint32_t*>(buf.getData().data()) == -1)
        {
            handleUnconnectedPacket(buf, fromAddr);
        }
    }
}

void Server::handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr)
{
    //read the -1 header
    {
        uint32_t byte;
        buf.readUint32(byte);
    }

    std::string str;
    if (!buf.readString(str)) //couldn't read str
    {
        return;
    }

    if (str == "connect")
    {
    }
}

void Server::handleEvents()
{

}

void Server::tryRunTicks()
{

}
