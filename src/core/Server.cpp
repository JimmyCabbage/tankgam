#include "Server.h"

#include <fmt/format.h>

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
        
        entityManager = std::make_unique<EntityManager>();
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
        
        sendPackets();
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

EntityId Server::allocateGlobalEntity(Entity globalEntity)
{
    EntityId netEntityId = entityManager->allocateGlobalEntity();
    Entity* newEntity = entityManager->getGlobalEntity(netEntityId);
    *newEntity = globalEntity;
    
    NetBuf sendBuf{};
    sendBuf.writeUint16(netEntityId);
    Entity::serialize(*newEntity, sendBuf);
    
    for (auto& client : clients)
    {
        if (client.state == ServerClientState::Free)
        {
            continue;
        }
        
        client.netChan->addReliableData(std::move(sendBuf), NetMessageType::CreateEntity);
    }
    
    return netEntityId;
}

void Server::freeGlobalEntity(EntityId netEntityId)
{
    entityManager->freeGlobalEntity(netEntityId);
    
    NetBuf sendBuf{};
    sendBuf.writeUint16(netEntityId);
    
    for (auto& client : clients)
    {
        if (client.state != ServerClientState::Free)
        {
            continue;
        }
        
        client.netChan->addReliableData(std::move(sendBuf), NetMessageType::DestroyEntity);
    }
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
        if (!theClient->netChan->processHeader(buf, msgType, reliableMessages) ||
            msgType == NetMessageType::Unknown)
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

        if (msgType == NetMessageType::SendReliables)
        {
            continue;
        }
        
        handleUnreliablePacket(buf, msgType, *theClient);
    }

    for (auto& client : clients)
    {
        if (client.state == ServerClientState::Free)
        {
            continue;
        }

        //if we don't send any unreliable info
        client.netChan->trySendReliable();
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
    if (msgType == NetMessageType::Synchronize)
    {
        uint64_t clientTime;
        if (!buf.readUint64(clientTime))
        {
            return;
        }

        NetBuf sendBuf{};
        sendBuf.writeUint64(clientTime);
        sendBuf.writeUint64(timer->getTotalTicks());
        
        std::vector<EntityId> globalEntities = entityManager->getGlobalEntities();
        sendBuf.writeUint32(static_cast<uint32_t>(globalEntities.size()));
        for (EntityId entityId : globalEntities)
        {
            Entity* globalEntity = entityManager->getGlobalEntity(entityId);
            if (!globalEntity)
            {
                throw std::runtime_error{ "This should never happen (global entity not available?!)" };
            }
            
            sendBuf.writeUint16(entityId);
            Entity::serialize(*globalEntity, sendBuf);
        }
        
        theClient.netChan->addReliableData(std::move(sendBuf), NetMessageType::Synchronize);
        
        allocateGlobalEntity(Entity{ glm::vec3{ 0.0f, -2.5f, -7.0f }, glm::identity<glm::quat>(), "models/tank/tank_body.txt" });
        allocateGlobalEntity(Entity{ glm::vec3{ 0.0f, -2.5f, -7.0f }, glm::identity<glm::quat>(), "models/tank/tank_turret.txt" });
        bleh = true;
    }
}

void Server::handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& theClient)
{

}

void Server::handleEvents()
{

}

void Server::tryRunTicks()
{
#if 0
    const uint64_t totalTicks = timer->getTotalTicks();
    const uint64_t ticks = totalTicks - lastTick;
    
    if (ticks == 0)
    {
        return;
    }
    
    for (uint64_t i = 0; i < ticks; i++)
    {
        currentTick = lastTick + i;
        //for (auto& [id, thing] : things)
        {
            //    thing->think();
        }
    }
    
    lastTick = totalTicks;
#endif
    
    if (bleh)
    {
        Entity* entity = entityManager->getGlobalEntity(0);
        glm::mat4 d = glm::rotate(glm::mat4{1.0f}, glm::radians(std::fmod((float)timer->getTotalTicks(), 360.0f)), glm::vec3{0.0f, 1.0f, 0.0f});
        entity->rotation = glm::quat_cast(d);
    }
}

void Server::sendPackets()
{
    for (auto& client : clients)
    {
        if (client.state == ServerClientState::Free)
        {
            continue;
        }
        
        NetBuf sendBuf{};
        
        const std::vector<EntityId> globalEntities = entityManager->getGlobalEntities();
        sendBuf.writeUint32(static_cast<uint32_t>(globalEntities.size()));
        for (EntityId globalId : globalEntities)
        {
            sendBuf.writeUint16(globalId);
            
            Entity* entity = entityManager->getGlobalEntity(globalId);
            Entity::serialize(*entity, sendBuf);
        }
        
        client.netChan->sendData(std::move(sendBuf), NetMessageType::EntitySynchronize);
    }
}
