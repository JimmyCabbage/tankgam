#include "Server.h"

#include "sys/Console.h"
#include "sys/File.h"
#include "sys/Timer.h"
#include "sys/Net.h"
#include "sys/NetChan.h"

Server::Server(Console& console, FileManager& fileManager, Net& net)
    : console{ console }, fileManager{ fileManager }, net{ net }
{
    try
    {
        clients.resize(1);
        for (auto& client : clients)
        {
            client.state = ServerClientState::Free;
            client.netChan = std::make_unique<NetChan>(net, NetSrc::Server);
        }

        console.log("Server: Init Timer Subsystem...");
        timer = std::make_unique<Timer>();
        timer->start();
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
            continue;
        }

        ServerClient* theClient = nullptr;
        for (auto& client : clients)
        {
            if (client.state == ServerClientState::Free)
            {
                continue;
            }

            if (client.netChan->getToAddr() != fromAddr)
            {
                continue;
            }

            theClient = &client;
        }

        const NetMessageType msgType = theClient->netChan->processHeader(buf);
        if (msgType == NetMessageType::Unknown)
        {
            continue;
        }

        if (msgType == NetMessageType::Time)
        {
            uint64_t clientTime;
            buf.readUint64(clientTime);

            NetBuf sendBuf{};
            sendBuf.writeUint64(clientTime);
            sendBuf.writeUint64(timer->getTotalTicks());

            theClient->netChan->sendData(std::move(sendBuf), NetMessageType::Time);
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

    if (str == "client_connect")
    {
        ServerClient* clientNew = nullptr;
        for (auto& client : clients)
        {
            if (client.state != ServerClientState::Free)
            {
                continue;
            }

            clientNew = &client;
            break;
        }

        if (!clientNew)
        {
            return;
        }

        clientNew->state = ServerClientState::Connected;
        clientNew->netChan->setToAddr(fromAddr);

        clientNew->netChan->outOfBandPrint(fromAddr, "server_connect");
    }
}

void Server::handleEvents()
{

}

void Server::tryRunTicks()
{

}
