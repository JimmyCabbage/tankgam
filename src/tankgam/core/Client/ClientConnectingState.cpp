#include "Client/ClientConnectingState.h"

#include <random>

#include "sys/Timer.h"
#include "sys/Renderer.h"
#include "Client/ClientConnectedState.h"
#include "Client.h"
#include "sys/Console.h"
#include "Net.h"
#include "NetChan.h"
#include "Entity.h"
#include "EntityManager.h"

ClientConnectingState::ClientConnectingState(Client& client, Renderer& renderer, Log& log, Net& net, NetAddr serverAddr)
    : client{ client }, renderer{ renderer }, log{ log }, net{ net }, serverAddr{ serverAddr }
{
    netChan = std::make_unique<NetChan>(net, NetSrc::Client);

    timer = std::make_unique<Timer>();
    timer->start();
    stopTryConnectTick = timer->getTotalTicks() + Timer::TICK_RATE * 30;

    connectState = ConnectState::Connecting;

    //generate a random client connection salt
    {
        std::random_device dev;
        std::mt19937 rng{ dev() };
        std::uniform_int_distribution<std::mt19937::result_type> dist{ 1 };

        clientSalt = dist(rng);
    }
    nextSendTick = 0;
    trySendConnectionRequest();

    //zero is an invalid salt
    serverSalt = 0;
    combinedSalt = 0;
}

ClientConnectingState::~ClientConnectingState() = default;

void ClientConnectingState::pause()
{
}

void ClientConnectingState::resume()
{
    if (connectState == ConnectState::GiveUp ||
        connectState == ConnectState::Connected)
    {
        client.popState();
    }
}

bool ClientConnectingState::consumeEvent(const Event& ev)
{
    return false;
}

void ClientConnectingState::update()
{
    if (connectState == ConnectState::GiveUp) //if we want to die, die
    {
        client.popState();
        return;
    }
    else if (connectState == ConnectState::Connected) //if we're already connected, don't do nothing
    {
        return;
    }

    //has it been long enough? die
    const uint64_t currentTick = timer->getTotalTicks();
    if (currentTick >= stopTryConnectTick)
    {
        connectState = ConnectState::GiveUp;
        return;
    }

    if (connectState == ConnectState::Connecting) //try to send another connection packet if it's been long enough
    {
        trySendConnectionRequest();
    }
    else if (connectState == ConnectState::Challenging) //try to send another challenge packet if it's been long enough
    {
        trySendChallengeRequest();
    }
    else if (connectState == ConnectState::AlmostConnected)
    {
        trySendSynchronizeRequest();
    }

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
            continue;
        }

        //reset read head
        buf.beginRead();

        if (connectState != ConnectState::AlmostConnected)
        {
            continue;
        }

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

            if (handleReliablePacket(reliableMessage, reliableMsgType))
            {
                return;
            }
        }

        if (msgType == NetMessageType::SendReliables)
        {
            continue;
        }

        handleUnreliablePacket(buf, msgType);
    }

    if (netChan)
    {
        //if we never sent a reliable
        netChan->trySendReliable(combinedSalt);
    }
}

void ClientConnectingState::handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr)
{
    std::string str;
    if (!buf.readString(str)) //couldn't read str
    {
        return;
    }

    log.logf(LogLevel::Debug, "Client: Unconnected packet: %s\n", str.c_str());

    if (str == "server_challenge")
    {
        if (connectState == ConnectState::Challenging)
        {
            return;
        }

        uint32_t clientSaltOfServer;
        if (!buf.readUint32(clientSaltOfServer))
        {
            return;
        }

        if (clientSaltOfServer != clientSalt)
        {
            log.logf("Client: Client & server's client salt does not match: %d vs %d\n",
                clientSalt, clientSaltOfServer);
            return;
        }

        if (!buf.readUint32(serverSalt))
        {
            return;
        }

        combinedSalt = clientSalt ^ serverSalt;
        connectState = ConnectState::Challenging;

        nextSendTick = 0;
        trySendChallengeRequest();
    }
    else if (str == "server_connect")
    {
        if (connectState == ConnectState::AlmostConnected)
        {
            return;
        }

        uint32_t mixedSalt;
        if (!buf.readUint32(mixedSalt))
        {
            return;
        }

        if (mixedSalt != combinedSalt)
        {
            return;
        }

        netChan->setToAddr(fromAddr);
        connectState = ConnectState::AlmostConnected;

        //prepare entity manager
        entityManager = std::make_unique<EntityManager>();

        nextSendTick = 0;
        trySendSynchronizeRequest();
    }
}

bool ClientConnectingState::handleReliablePacket(NetBuf& buf, const NetMessageType& msgType)
{
    if (msgType == NetMessageType::Synchronize)
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

        connectState = ConnectState::Connected;
        client.pushState(std::make_unique<ClientConnectedState>(client, renderer, log,
            net, serverAddr,
            std::move(netChan), std::move(timer),
            std::move(entityManager),
            std::move(models),
            clientSalt, serverSalt));

        return true;
    }

    return false;
}

void ClientConnectingState::handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType)
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

void ClientConnectingState::draw()
{
    if (connectState == ConnectState::GiveUp ||
        connectState == ConnectState::Connected)
    {
        return;
    }

    constexpr std::string_view CONNECT_MSG = "Connecting...";
    constexpr float SIZE = 32.0f;
    renderer.drawText(CONNECT_MSG, glm::vec2{ renderer.getWidth() / 2 - (SIZE * CONNECT_MSG.size() / 2), renderer.getHeight() / 2 }, SIZE);
}

void ClientConnectingState::trySendConnectionRequest()
{
    if (connectState != ConnectState::Connecting)
    {
        throw std::runtime_error{ "Cannot send connection request after connected" };
    }

    const uint64_t currentTick = timer->getTotalTicks();
    if (currentTick >= nextSendTick)
    {
        NetBuf sendBuf;
        sendBuf.writeString("client_connect");
        sendBuf.writeUint32(clientSalt);

        NetChan::outOfBand(net, NetSrc::Client, serverAddr, std::move(sendBuf));
        nextSendTick = currentTick + Timer::TICK_RATE * 5;

        log.log("Client: Attempted to send connection packet...\n");
    }
}

void ClientConnectingState::trySendChallengeRequest()
{
    if (connectState != ConnectState::Challenging)
    {
        throw std::runtime_error{ "Cannot send connection request after challenge recieved" };
    }

    const uint64_t currentTick = timer->getTotalTicks();
    if (currentTick >= nextSendTick)
    {
        NetBuf sendBuf;
        sendBuf.writeString("client_challenge");
        sendBuf.writeUint32(combinedSalt);

        NetChan::outOfBand(net, NetSrc::Client, serverAddr, std::move(sendBuf));
        nextSendTick = currentTick + Timer::TICK_RATE * 5;

        log.log("Client: Attempted to send challenge packet...\n");
    }
}

void ClientConnectingState::trySendSynchronizeRequest()
{
    if (connectState != ConnectState::AlmostConnected)
    {
        throw std::runtime_error{ "Cannot send sync request after already synchronized" };
    }

    const uint64_t currentTick = timer->getTotalTicks();
    if (currentTick >= nextSendTick)
    {
        //synchronize time
        const uint64_t oldClientTime = timer->getTotalTicks();

        NetBuf sendBuf{};
        sendBuf.writeUint64(oldClientTime);

        netChan->addReliableData(std::move(sendBuf), NetMessageType::Synchronize);
        nextSendTick = currentTick + Timer::TICK_RATE * 5;

        log.log("Client: Attempted to send synchronization packet...\n");
    }
}
