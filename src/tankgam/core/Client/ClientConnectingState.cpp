#include "Client/ClientConnectingState.h"

#include "sys/Timer.h"
#include "sys/Renderer.h"
#include "Client/ClientConnectedState.h"
#include "Client.h"
#include "Net.h"
#include "NetChan.h"
#include "Entity.h"
#include "EntityManager.h"

ClientConnectingState::ClientConnectingState(Net& net, NetAddr serverAddr)
    : net{ net }
{
    netChan = std::make_unique<NetChan>(net, NetSrc::Client);
    netChan->outOfBandPrint(serverAddr, "client_connect");

    timer = std::make_unique<Timer>();
    timer->start();
    stopTryConnectTick = timer->getTotalTicks() + Timer::TICK_RATE * 6;

    connected = false;
}

ClientConnectingState::~ClientConnectingState() = default;

void ClientConnectingState::pause()
{
}

void ClientConnectingState::resume()
{
}

bool ClientConnectingState::consumeEvent(const Event& ev)
{
    return false;
}

void ClientConnectingState::update(Client& client, Renderer& renderer)
{
    NetBuf buf{};
    NetAddr fromAddr{};
    while (netChan && net.getPacket(NetSrc::Client, buf, fromAddr))
    {
        //read the first byte of the msg
        //if it's -1 then it's an unconnected message
        if (*reinterpret_cast<const uint32_t*>(buf.getData().data()) == -1)
        {
            handleUnconnectedPacket(buf, fromAddr);
            continue;
        }

        NetMessageType msgType;
        std::vector<NetBuf> reliableMessages;
        if (!netChan->processHeader(buf, msgType, reliableMessages) ||
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
                    continue;
                }

                reliableMsgType = static_cast<NetMessageType>(tempV);
            }

            handleReliablePacket(client, renderer, reliableMessage, reliableMsgType);
        }

        if (msgType == NetMessageType::SendReliables)
        {
            continue;
        }

        handleUnreliablePacket(client, renderer, buf, msgType);
    }

    if (netChan)
    {
        //if we never sent a reliable
        netChan->trySendReliable();
    }
}

void ClientConnectingState::handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr)
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

    if (str == "server_connect")
    {
        netChan->setToAddr(fromAddr);

        //synchronize time
        const uint64_t oldClientTime = timer->getTotalTicks();

        NetBuf sendBuf{};
        sendBuf.writeUint64(oldClientTime);

        netChan->addReliableData(std::move(sendBuf), NetMessageType::Synchronize);

        //prepare entity manager
        entityManager = std::make_unique<EntityManager>();
    }
}

void ClientConnectingState::handleReliablePacket(Client& client, Renderer& renderer, NetBuf& buf, const NetMessageType& msgType)
{
    if (!entityManager || !timer)
    {
        return;
    }

    if (msgType == NetMessageType::Synchronize && !connected)
    {
        //time stuff
        uint64_t prevTime;
        buf.readUint64(prevTime);

        uint64_t serverTime;
        buf.readUint64(serverTime);

        const uint64_t roundTripTime = (timer->getTotalTicks() - prevTime);
        timer->setTickOffset(serverTime + (roundTripTime / 2) + 1);

        uint32_t numEntities;
        buf.readUint32(numEntities);
        for (uint32_t i = 0; i < numEntities; i++)
        {
            EntityId netEntityId;
            buf.readUint16(netEntityId);

            if (!entityManager->doesEntityExist(netEntityId))
            {
                entityManager->allocateGlobalEntity(netEntityId);
            }

            Entity* entity = entityManager->getGlobalEntity(netEntityId);
            if (!entity)
            {
                throw std::runtime_error{ "This should never happen (global entity not available?!)" };
            }

            Entity::deserialize(*entity, buf);

            if (!models.contains(entity->modelName))
            {
                models[entity->modelName] = renderer.createModel(entity->modelName);
            }
        }

        connected = true;
        client.pushState(std::make_shared<ClientConnectedState>(net, std::move(netChan), std::move(timer),
            std::move(entityManager),
            std::move(models)));

        //hideMenu();
    }
    else if (msgType == NetMessageType::CreateEntity)
    {
        EntityId netEntityId;
        buf.readUint16(netEntityId);

        entityManager->allocateGlobalEntity(netEntityId);
        Entity* newEntity = entityManager->getGlobalEntity(netEntityId);

        Entity::deserialize(*newEntity, buf);

        if (!models.contains(newEntity->modelName))
        {
            models[newEntity->modelName] = renderer.createModel(newEntity->modelName);
        }
    }
    else if (msgType == NetMessageType::DestroyEntity)
    {
        EntityId netEntityId;
        buf.readUint16(netEntityId);

        entityManager->freeGlobalEntity(netEntityId);
    }
}

void ClientConnectingState::handleUnreliablePacket(Client& client, Renderer& renderer, NetBuf& buf, const NetMessageType& msgType)
{
    if (!entityManager)
    {
        return;
    }

    if (msgType == NetMessageType::EntitySynchronize)
    {
        uint32_t numEntities;
        buf.readUint32(numEntities);

        for (uint32_t i = 0; i < numEntities; i++)
        {
            EntityId globalId;
            buf.readUint16(globalId);

            Entity* entity = entityManager->getGlobalEntity(globalId);
            if (!entity)
            {
                entityManager->allocateGlobalEntity(globalId);
                entity = entityManager->getGlobalEntity(globalId);
            }

            Entity::deserialize(*entity, buf);

            if (!models.contains(entity->modelName))
            {
                models[entity->modelName] = renderer.createModel(entity->modelName);
            }
        }
    }
}

void ClientConnectingState::draw(Renderer& renderer)
{
    constexpr std::string_view CONNECT_MSG = "Connecting...";
    constexpr float SIZE = 32.0f;
    renderer.drawText(CONNECT_MSG, glm::vec2{ renderer.getWidth() / 2 - (SIZE * CONNECT_MSG.size() / 2), renderer.getHeight() / 2 }, SIZE);
}

bool ClientConnectingState::isFinished()
{
    return !connected;
}
