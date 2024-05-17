#include "Server.h"

#include <random>

#include <fmt/format.h>

#include "sys/Console.h"
#include "sys/Timer.h"
#include "Net.h"
#include "NetChan.h"
#include "NetBuf.h"

#include <util/FileManager.h>

Server::Server(Log& log, FileManager& fileManager, Net& net)
    : log{ log }, fileManager{ fileManager }, net{ net }
{
    try
    {
        clients.resize(4);
        for (auto& client : clients)
        {
            client.state = ServerClientState::Free;
            client.netChan = std::make_unique<NetChan>(net, NetSrc::Server);
            client.lastRecievedTime = 0;
            client.clientSalt = 0;
            client.serverSalt = 0;
            client.combinedSalt = 0;
        }

        log.log("Server: Init Timer Subsystem...");
        timer = std::make_unique<Timer>();
        timer->start();
        
        entityManager = std::make_unique<EntityManager>();
        
        allocateGlobalEntity(Entity{ glm::vec3{ 0.0f, -2.5f, -7.0f }, glm::identity<glm::quat>(), "models/tank/tank_body.txt" });
        allocateGlobalEntity(Entity{ glm::vec3{ 0.0f, -2.5f, -7.0f }, glm::identity<glm::quat>(), "models/tank/tank_turret.txt" });
    }
    catch (const std::exception& e)
    {
        log.log(LogLevel::Error, fmt::format("Server: Init Error:\n{}", e.what()));
        throw;
    }

    running = true;

    log.log("Server: Initialized");
}

Server::~Server()
{
    log.log("Server: Quitting");

    for (auto& client : clients)
    {
        if (client.state != ServerClientState::Connected)
        {
            continue;
        }

        //disconnect all clients
        const bool forceDisconnect = client.state == ServerClientState::Connected ||
                client.state == ServerClientState::Spawned;

        disconnectClient(client, forceDisconnect);
    }
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
        log.log(LogLevel::Error, fmt::format("Server: Runtime Error:\n{}", e.what()));
        throw;
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
    
    for (auto& client : clients)
    {
        if (client.state == ServerClientState::Free)
        {
            continue;
        }
        
        NetBuf sendBuf{};
        sendBuf.writeUint16(netEntityId);
        Entity::serialize(*newEntity, sendBuf);

        client.netChan->addReliableData(std::move(sendBuf), NetMessageType::CreateEntity);
    }
    
    return netEntityId;
}

void Server::freeGlobalEntity(EntityId netEntityId)
{
    entityManager->freeGlobalEntity(netEntityId);
    
    for (auto& client : clients)
    {
        if (client.state != ServerClientState::Free)
        {
            continue;
        }
        
        NetBuf sendBuf{};
        sendBuf.writeUint16(netEntityId);
        
        client.netChan->addReliableData(std::move(sendBuf), NetMessageType::DestroyEntity);
    }
}

void Server::disconnectClient(ServerClient& client, bool forceDisconnect)
{
    log.logf("Server: Disconnect client from %d", (int)client.netChan->getToAddr().port);

    if (forceDisconnect)
    {
        NetBuf sendBuf;
        sendBuf.writeString("server_disconnect");
        sendBuf.writeUint32(client.combinedSalt);

        NetChan::outOfBand(net, NetSrc::Server, client.netChan->getToAddr(), std::move(sendBuf));
    }

    client.state = ServerClientState::Free;
    client.netChan = std::make_unique<NetChan>(net, NetSrc::Server);
    client.lastRecievedTime = 0;
    client.clientSalt = 0;
    client.serverSalt = 0;
    client.combinedSalt = 0;
}

void Server::handlePackets()
{
    NetBuf buf{};
    NetAddr fromAddr{};
    while (net.getPacket(NetSrc::Server, buf, fromAddr))
    {
        //read the first 2 bytes of the msg
        uint16_t header;
        if (!buf.readUint16(header))
        {
            continue;
        }

        //if it matches the out of band then it's an unconnected message
        if (header == NetChan::OUT_OF_BAND_MAGIC_NUMBER)
        {
            handleUnconnectedPacket(buf, fromAddr);
            continue;
        }

        //if it's not reliable magic number then it's broken
        if (header != NetChan::RELIABLE_MAGIC_NUMBER)
        {
            continue;
        }

        //reset read head
        buf.beginRead();

        ServerClient* client = nullptr;
        for (auto& currentClient : clients)
        {
            if (currentClient.state == ServerClientState::Free ||
                currentClient.netChan->getToAddr() != fromAddr)
            {
                continue;
            }

            client = &currentClient;
        }

        if (!client)
        {
            continue;
        }
        
        client->lastRecievedTime = timer->getTotalTicks();
        
        NetMessageType msgType = NetMessageType::Unknown;
        std::vector<NetBuf> reliableMessages;
        if (!client->netChan->processHeader(buf, msgType, reliableMessages, client->combinedSalt) ||
            msgType == NetMessageType::Unknown)
        {
            continue;
        }

        for (auto& reliableMessage : reliableMessages)
        {
            //reliable messages have their own type
            NetMessageType reliableMsgType;
            {
                uint8_t tempV;
                if (!reliableMessage.readUint8(tempV))
                {
                    log.logf(LogLevel::Warning, "Unknown type of %d", tempV);
                    continue;
                }

                reliableMsgType = static_cast<NetMessageType>(tempV);
            }

            handleReliablePacket(reliableMessage, reliableMsgType, *client);
        }

        if (msgType == NetMessageType::SendReliables)
        {
            continue;
        }
        
        handleUnreliablePacket(buf, msgType, *client);
    }

    for (auto& client : clients)
    {
        if (client.state == ServerClientState::Free)
        {
            continue;
        }

        //if we don't send any unreliable info
        client.netChan->trySendReliable(client.combinedSalt);

        //client timeout
        if (client.lastRecievedTime + Timer::TICK_RATE * 30 < timer->getTotalTicks())
        {
            const bool forceDisconnect = client.state == ServerClientState::Connected ||
                client.state == ServerClientState::Spawned;

            disconnectClient(client, forceDisconnect);
            continue;
        }
    }
}

void Server::handleUnconnectedPacket(NetBuf& buf, const NetAddr& fromAddr)
{
    std::string str;
    if (!buf.readString(str)) //couldn't read str
    {
        return;
    }

    log.logf(LogLevel::Debug, "Server: Unconnected packet: %s\n", str.c_str());

    if (str == "client_connect")
    {
        uint32_t clientSalt;
        if (!buf.readUint32(clientSalt))
        {
            return;
        }

        //check if client already exists
        for (auto& client : clients)
        {
            if (client.state != ServerClientState::Free &&
                client.clientSalt == clientSalt)
            {
                log.logf("Server: Client tried to connect from %d twice", (int)fromAddr.port);
                return;
            }
        }

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
            NetChan::outOfBandPrint(net, NetSrc::Server, fromAddr, "server_noroom");
            return;
        }

        newClient->state = ServerClientState::Challenging;
        newClient->netChan->setToAddr(fromAddr);
        newClient->lastRecievedTime = timer->getTotalTicks();

        //fill out salts
        newClient->clientSalt = clientSalt;
        log.logf(LogLevel::Debug, "Server: Recieved client salt of %d", newClient->clientSalt);
        {
            std::random_device dev;
            std::mt19937 rng{ dev() };
            std::uniform_int_distribution<std::mt19937::result_type> dist{ 1 };

            newClient->serverSalt = dist(rng);
        }
        log.logf(LogLevel::Debug, "Server: Chose server salt of %d", newClient->serverSalt); 
    
        newClient->combinedSalt = newClient->clientSalt ^ newClient->serverSalt;
        log.logf(LogLevel::Debug, "Server: Combined salt of %d", newClient->combinedSalt); 

        //send back a challenge
        {
            NetBuf sendBuf;
            sendBuf.writeString("server_challenge");
            sendBuf.writeUint32(newClient->clientSalt);
            sendBuf.writeUint32(newClient->serverSalt);

            NetChan::outOfBand(net, NetSrc::Server, fromAddr, std::move(sendBuf));
        }
        
        log.logf("Server: New client challenging from %d", (int)fromAddr.port);
    }
    else if (str == "client_challenge")
    {
        uint32_t combinedSalt;
        if (!buf.readUint32(combinedSalt))
        {
            return;
        }

        ServerClient* newClient = nullptr;
        for (auto& client : clients)
        {
            if (client.state != ServerClientState::Challenging)
            {
                continue;
            }

            if (combinedSalt == client.combinedSalt)
            {
                newClient = &client;
                break;
            }
        }

        if (!newClient)
        {
            return;
        }

        newClient->state = ServerClientState::Connected;
        newClient->netChan->setToAddr(fromAddr);
        newClient->lastRecievedTime = timer->getTotalTicks();

        {
            NetBuf sendBuf;
            sendBuf.writeString("server_connect");
            sendBuf.writeUint32(newClient->combinedSalt);

            NetChan::outOfBand(net, NetSrc::Server, fromAddr, std::move(sendBuf));
        }

        log.logf("Server: New client connecting from %d", (int)fromAddr.port);
    }
    else if (str == "client_disconnect")
    {
        uint32_t combinedSalt;
        if (!buf.readUint32(combinedSalt))
        {
            return;
        }

        ServerClient* disconnectingClient = nullptr;
        for (auto& client : clients)
        {
            if (client.state == ServerClientState::Free || client.state == ServerClientState::Challenging)
            {
                continue;
            }

            if (combinedSalt == client.combinedSalt)
            {
                disconnectingClient = &client;
                break;
            }
        }

        if (!disconnectingClient)
        {
            return;
        }

        disconnectClient(*disconnectingClient, false);
    }
}

void Server::handleReliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& client)
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

        client.netChan->addReliableData(std::move(sendBuf), NetMessageType::Synchronize);
    }
}

void Server::handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType, ServerClient& client)
{
    if (msgType == NetMessageType::PlayerCommand)
    {
        float rot = 0.0f;
        buf.readFloat(rot);

        rotationAmount += rot;

        log.logf(LogLevel::Debug, "Server: Rotation command recieved from: %d (tick %d)", client.netChan->getToAddr().port, timer->getTotalTicks());
    }
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
    
    Entity* entity = entityManager->getGlobalEntity(0);
    glm::mat4 d = glm::rotate(glm::mat4{1.0f}, glm::radians(std::fmod(rotationAmount, 360.0f)), glm::vec3{0.0f, 1.0f, 0.0f});
    entity->rotation = glm::quat_cast(d);
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
        
        client.netChan->sendData(std::move(sendBuf), NetMessageType::EntitySynchronize, client.combinedSalt);
    }
}
