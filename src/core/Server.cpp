#include "Server.h"

#include "sys/Console.h"
#include "sys/File.h"
#include "sys/Timer.h"
#include "sys/Net.h"
#include "NetChan.h"
#include "NetBuf.h"

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

        if (!theClient)
        {
            continue;
        }

        NetMessageType msgType = NetMessageType::Unknown;
        std::vector<NetBuf> reliableMessages;
        if (!theClient->netChan->processHeader(buf, msgType, reliableMessages))
        {
            continue;
        }

        for (auto& reliableMessage : reliableMessages)
        {
            //reliable messages have their own type
            NetMessageType reliableMsgType = NetMessageType::Unknown;
            {
                uint8_t tempV;
                if (!reliableMessage.readUint8(tempV))
                {
                    continue;
                }

                reliableMsgType = static_cast<NetMessageType>(tempV);
            }

            handleReliablePacket(reliableMessage, reliableMsgType, *theClient);
        }

        //just to make sure reliable messages get sent (for now)
        theClient->netChan->sendData(std::span<const std::byte>{}, NetMessageType::Unknown);

        if (msgType == NetMessageType::Unknown)
        {
            continue;
        }

        //TODO: handle unreliable messages here
    }
}

void Server::handleUnconnectedPacket(NetBuf& buf, const NetAddr& fromAddr)
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
        ServerClient* newClient = nullptr;
        for (auto& client : clients)
        {
            if (client.state != ServerClientState::Free)
            {
                continue;
            }

            newClient = &client;
            break;
        }

        if (!newClient)
        {
            return;
        }

        newClient->state = ServerClientState::Connected;
        newClient->netChan->setToAddr(fromAddr);

        newClient->netChan->outOfBandPrint(fromAddr, "server_connect");
    }
}

void Server::handleReliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& theClient)
{
    if (msgType == NetMessageType::Time)
    {
        uint64_t clientTime;
        if (!buf.readUint64(clientTime))
        {
            return;
        }

        NetBuf sendBuf{};
        sendBuf.writeUint64(clientTime);
        sendBuf.writeUint64(timer->getTotalTicks());

        theClient.netChan->addReliableData(std::move(sendBuf), NetMessageType::Time);
    }
}

void Server::handleEvents()
{

}

void Server::tryRunTicks()
{

}
