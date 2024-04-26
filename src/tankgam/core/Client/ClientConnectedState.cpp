#include "Client/ClientConnectedState.h"

#include "sys/Renderer.h"
#include "sys/Timer.h"
#include "Client.h"
#include "NetChan.h"
#include "EntityManager.h"

ClientConnectedState::ClientConnectedState(Net& net, std::unique_ptr<NetChan> netChan, std::unique_ptr<Timer> timer,
        std::unique_ptr<EntityManager> entityManager,
        std::unordered_map<std::string, std::unique_ptr<Model>> models)
    : net{ net },
      netChan{ std::move(netChan) }, timer{ std::move(timer) },
      entityManager{ std::move(entityManager) },
      models{ std::move(models) }
{
}

ClientConnectedState::~ClientConnectedState() = default;

void ClientConnectedState::pause()
{
}

void ClientConnectedState::resume()
{
}

bool ClientConnectedState::consumeEvent(const Event& ev)
{
    switch (ev.type)
    {
    case EventType::KeyDown:
        switch (static_cast<KeyPressType>(ev.data1))
        {
        case KeyPressType::DownArrow:
            commands.emplace(PlayerCommand{ 5.0f });
            break;
        case KeyPressType::UpArrow:
            commands.emplace(PlayerCommand{ -5.0f });
            break;
        case KeyPressType::Escape:
            disconnect();
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }

    return true;
}


void ClientConnectedState::update(Client& client, Renderer& renderer)
{
    NetBuf buf{};
    NetAddr fromAddr{};
    while (net.getPacket(NetSrc::Client, buf, fromAddr))
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

    //if we never sent a reliable
    netChan->trySendReliable();

    sendPackets();
}

void ClientConnectedState::handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr)
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

void ClientConnectedState::handleReliablePacket(Client& client, Renderer& renderer, NetBuf& buf, const NetMessageType& msgType)
{
    if (!entityManager || !timer)
    {
        return;
    }

    if (msgType == NetMessageType::CreateEntity)
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

void ClientConnectedState::handleUnreliablePacket(Client& client, Renderer& renderer, NetBuf& buf, const NetMessageType& msgType)
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

void ClientConnectedState::sendPackets()
{
    while (!commands.empty())
    {
        PlayerCommand& cmd = commands.front();
        commands.pop();

        NetBuf buf{};
        buf.writeFloat(cmd.addRotation);

        netChan->sendData(std::move(buf), NetMessageType::PlayerCommand);
    }
}


void ClientConnectedState::draw(Renderer& renderer)
{
    auto entities = entityManager->getGlobalEntities();
    for (EntityId entity : entities)
    {
        const Entity* e = entityManager->getGlobalEntity(entity);
        renderer.drawModel(*models[e->modelName], glm::vec3{1.0f}, e->rotation, e->position);
    }
}

bool ClientConnectedState::isFinished()
{
    return false;
}

void ClientConnectedState::disconnect()
{
    netChan->sendData(NetBuf{}, NetMessageType::Disconnect);

    timer->stop();

    netChan = std::make_unique<NetChan>(net, NetSrc::Client);
}
