#include "Client/ClientConnectedState.h"

#include "sys/Renderer.h"
#include "sys/Timer.h"
#include "sys/Console.h"
#include "Client.h"
#include "NetChan.h"
#include "EntityManager.h"

ClientConnectedState::ClientConnectedState(Client& client, Renderer& renderer, Console& console,
        Net& net, NetAddr serverAddr,
        std::unique_ptr<NetChan> netChan, std::unique_ptr<Timer> timer,
        std::unique_ptr<EntityManager> entityManager,
        std::unordered_map<std::string, std::unique_ptr<Model>> models,
        uint32_t clientSalt, uint32_t serverSalt)
    : client{ client }, renderer{ renderer }, console{ console },
      net{ net }, serverAddr{ serverAddr },
      netChan{ std::move(netChan) }, timer{ std::move(timer) },
      entityManager{ std::move(entityManager) },
      models{ std::move(models) },
      clientSalt{ clientSalt },
      serverSalt{ serverSalt },
      combinedSalt{ clientSalt ^ serverSalt }
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


void ClientConnectedState::update()
{
    NetBuf buf{};
    NetAddr fromAddr{};
    while (net.getPacket(NetSrc::Client, buf, fromAddr))
    {
        //read the first byte of the msg
        //if it's -1 then it's an unconnected message
        uint32_t header;
        if (!buf.readUint32(header))
        {
            continue;
        }

        if (header == -1)
        {
            handleUnconnectedPacket(buf, fromAddr);
            continue;
        }

        //reset read head
        buf.beginRead();

        NetMessageType msgType;
        std::vector<NetBuf> reliableMessages;
        if (!netChan->processHeader(buf, msgType, reliableMessages, combinedSalt) ||
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

            handleReliablePacket(reliableMessage, reliableMsgType);
        }

        if (msgType == NetMessageType::SendReliables)
        {
            continue;
        }

        handleUnreliablePacket(buf, msgType);
    }

    sendPackets();

    //if we never sent a reliable
    netChan->trySendReliable(combinedSalt);
}

void ClientConnectedState::handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr)
{
    std::string str;
    if (!buf.readString(str)) //couldn't read str
    {
        return;
    }

    console.logf("Client: Unconnected packet: %s\n", str.c_str());
}

void ClientConnectedState::handleReliablePacket(NetBuf& buf, const NetMessageType& msgType)
{
    if (msgType == NetMessageType::CreateEntity)
    {
        //console.log("CREATE");
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
        //console.log("DESTROY");
        EntityId netEntityId;
        buf.readUint16(netEntityId);

        entityManager->freeGlobalEntity(netEntityId);
    }
}

void ClientConnectedState::handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType)
{
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

        NetBuf sendBuf{};
        sendBuf.writeFloat(cmd.addRotation);

        netChan->sendData(std::move(sendBuf), NetMessageType::PlayerCommand, combinedSalt);
    }
}


void ClientConnectedState::draw()
{
    auto entities = entityManager->getGlobalEntities();
    for (EntityId entity : entities)
    {
        const Entity* e = entityManager->getGlobalEntity(entity);
        renderer.drawModel(*models[e->modelName], glm::vec3{1.0f}, e->rotation, e->position);
    }
}

void ClientConnectedState::disconnect()
{
    //just shoot off a bunch of disconnect packets, hope one of them reaches
    for (int i = 0; i < 3; i++)
    {
        NetBuf sendBuf;
        sendBuf.writeString("client_disconnect");
        sendBuf.writeUint32(combinedSalt);

        NetChan::outOfBand(net, NetSrc::Client, serverAddr, std::move(sendBuf));
    }

    timer->stop();

    client.popState();
}
