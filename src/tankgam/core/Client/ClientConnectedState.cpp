#include "Client/ClientConnectedState.h"

#include "sys/Renderer.h"
#include "sys/Timer.h"
#include "sys/Console.h"
#include "Client.h"
#include "NetChan.h"
#include "EntityManager.h"

ClientConnectedState::ClientConnectedState(Client& client, Renderer& renderer, Log& log,
        Net& net, NetAddr serverAddr,
        std::unique_ptr<NetChan> netChan, std::unique_ptr<Timer> timer,
        std::unique_ptr<EntityManager> entityManager,
        std::unordered_map<std::string, std::unique_ptr<Model>> models,
        uint32_t clientSalt, uint32_t serverSalt)
    : client{ client }, renderer{ renderer }, log{ log },
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
			log.logf(LogLevel::Warning, "Client: Recieved incorrect magic number of %d", header);
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
                    log.logf(LogLevel::Warning, "Client: Unknown message type of %d", tempV);
                    continue;
                }

                reliableMsgType = static_cast<NetMessageType>(tempV);
            }

            //log.logf(LogLevel::Debug, "Client: Recieved Reliable msg from server: %s", NetMessageTypeToString(reliableMsgType));

            handleReliablePacket(reliableMessage, reliableMsgType);
        }

        if (msgType == NetMessageType::SendReliables)
        {
            continue;
        }

        //log.logf(LogLevel::Debug, "Client: Recieved Unreliable msg from server: %s", NetMessageTypeToString(msgType));
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

    log.logf(LogLevel::Debug, "Client: Unconnected packet: %s\n", str.c_str());
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

        log.logf(LogLevel::Debug, "Client: Sent rotation command from on tick %d", timer->getTotalTicks());
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
