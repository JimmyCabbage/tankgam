#include "Client.h"

#include <stdexcept>
#include <numeric>

#include <fmt/format.h>

#include "sys/Console.h"
#include "sys/File.h"
#include "sys/EventHandler.h"
#include "sys/Renderer.h"
#include "sys/Timer.h"
#include "sys/Net.h"
#include "NetChan.h"
#include "NetBuf.h"
#include "Menu.h"
#include "Event.h"
#include "EntityManager.h"

Client::Client(Console& console, FileManager& fileManager, Net& net)
    : console{ console }, fileManager{ fileManager }, net{ net }
{
    clientState = ClientState::Disconnected;

    try
    {
        netChan = std::make_unique<NetChan>(net, NetSrc::Client);

        console.log("Client: Init Event Subsystem...");
        eventQueue = std::make_unique<EventQueue>();
        eventHandler = std::make_unique<EventHandler>(*eventQueue);

        console.log("Client: Init Renderer Subsystem...");
        renderer = std::make_unique<Renderer>(console, fileManager, "src");

        console.log("Client: Init Timer Subsystem...");
        timer = std::make_unique<Timer>();

        console.log("Client: Init Menu Subsystem...");
        menu = std::make_unique<Menu>(*renderer);

        const auto mainListCallback = [this](size_t choice) -> void
        {
            switch (choice)
            {
            case 0:
                connectToServer(NetAddr{ NetAddrType::Loopback });
                break;
            case 1:
                shutdown();
                break;
            }
        };
        MenuList mainList{ mainListCallback };
        mainList.addChoice("Start Game");
        mainList.addChoice("End Game");
        menu->addList(std::move(mainList));
    }
    catch (const std::exception& e)
    {
        console.log(fmt::format("Client: Init Error:\n{}", e.what()));
        throw e;
    }

    running = true;

    menuVisible = true;

    console.log("Client: Initialized");
}

Client::~Client()
{
    console.log("Client: Quitting");
}

bool Client::runFrame()
{
    try
    {
        nextFrameSettings();
        
        handlePackets();

        handleEvents();

        //tryRunTicks();

        draw();
    }
    catch (const std::exception& e)
    {
        console.log(fmt::format("Client: Runtime Error:\n{}", e.what()));
        throw e;
    }

    return running;
}

void Client::shutdown()
{
    running = false;
}

void Client::showMenu()
{
    menuVisible = true;
}

void Client::hideMenu()
{
    menuVisible = false;
}

#if 0
void Client::changeState(ClientState state)
{
    if (clientState == state)
    {
        return;
    }

    switch (state)
    {
    case ClientState::Disconnected:
        timer->stop();
        console.log("Client: Changing state to Disconnected");
        menuVisible = false;
        clientState = state;
        break;
    case ClientState::Connected:
        timer->start();
        console.log("Client: Changing state to Connected");
        menuVisible = false;
        clientState = state;
        lastTick = timer->getPassedTicks();
        break;
    default:
        throw std::runtime_error{ "Tried to changeState to an unknown state" };
    }
}
#endif

EntityManager* Client::getEntityManager(uint32_t sequence)
{
    const size_t realSequence = static_cast<size_t>(sequence) % EntityManager::NUM_ENTITY_MANAGERS;
    if (entityManagerSequences[realSequence] == sequence)
    {
        return &*entityManagers[realSequence];
    }
    
    return nullptr;
}

EntityManager& Client::insertEntityManager(uint32_t sequence)
{
    const size_t realSequence = static_cast<size_t>(sequence) % EntityManager::NUM_ENTITY_MANAGERS;
    entityManagerSequences[realSequence] = sequence;
    
    if (EntityManager* otherManager = getEntityManager(sequence - 1); otherManager)
    {
        entityManagers[realSequence] = std::make_unique<EntityManager>(*otherManager);
        return *entityManagers[realSequence];
    }
    
    entityManagers[realSequence] = std::make_unique<EntityManager>();
    return *entityManagers[realSequence];
}

void Client::connectToServer(NetAddr serverAddr)
{
    clientState = ClientState::Connecting;
    netChan->outOfBandPrint(serverAddr, "client_connect");

    timer->start();
}

void Client::disconnect()
{
    //TODO: complete
    clientState = ClientState::Disconnected;

    timer->stop();
}

void Client::nextFrameSettings()
{
    if (clientState != ClientState::Connected)
    {
        return;
    }
    
    currentEntityManager++;
    insertEntityManager(currentEntityManager);
}

void Client::handlePackets()
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

        if (clientState == ClientState::Disconnected)
        {
            continue;
        }

        NetMessageType msgType = NetMessageType::Unknown;
        std::vector<NetBuf> reliableMessages;
        if (!netChan->processHeader(buf, msgType, reliableMessages) ||
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

            handleReliablePacket(reliableMessage, reliableMsgType);
        }

        if (msgType == NetMessageType::SendReliables)
        {
            continue;
        }
        
        handleUnreliablePacket(buf, msgType);
    }

    //if we never sent a reliable
    netChan->trySendReliable();
}

void Client::handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr)
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
        //ignore this connection packet if we're already connected to a server
        if (clientState == ClientState::Connected && clientState != ClientState::Connecting)
        {
            return;
        }

        netChan->setToAddr(fromAddr);

        //synchronize time
        const uint64_t oldClientTime = timer->getTotalTicks();

        NetBuf sendBuf{};
        sendBuf.writeUint64(oldClientTime);

        netChan->addReliableData(std::move(sendBuf), NetMessageType::Synchronize);
        
        //prepare entity managers
        lastAckedEntityManager = 0;
        currentEntityManager = 0;
        for (size_t i = 0; i < EntityManager::NUM_ENTITY_MANAGERS; i++)
        {
            entityManagerSequences[i] = 0;
            ackedEntityManagers[i] = false;
            entityManagers[i] = std::make_unique<EntityManager>();
        }
    }
}

void Client::handleReliablePacket(NetBuf& buf, const NetMessageType& msgType)
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
        
        //entity manager stuff
        buf.readUint32(currentEntityManager);
        EntityManager& entityManager = insertEntityManager(currentEntityManager);
        
        uint32_t numEntities;
        buf.readUint32(numEntities);
        for (uint32_t i = 0; i < numEntities; i++)
        {
            EntityId netEntityId;
            buf.readUint16(netEntityId);
            
            entityManager.allocateGlobalEntity(netEntityId);
            Entity* entity = entityManager.getGlobalEntity(netEntityId);
            if (!entity)
            {
                throw std::runtime_error{ "This should never happen (global entity not available?!)" };
            }
            
            Entity::deserialize(*entity, buf);
        }
        
        clientState = ClientState::Connected;
        
        hideMenu();
    }
    else if (msgType == NetMessageType::CreateEntity)
    {
        uint32_t entityManagerNum;
        buf.readUint32(entityManagerNum);
        
        EntityManager* entityManager = getEntityManager(entityManagerNum);
        if (!entityManager)
        {
            return;
        }
        
        EntityId netEntityId;
        buf.readUint16(netEntityId);
        
        entityManager->allocateGlobalEntity(netEntityId);
        Entity* newEntity = entityManager->getGlobalEntity(netEntityId);
        
        Entity::deserialize(*newEntity, buf);
        
        models[newEntity->modelName] = renderer->createModel(newEntity->modelName);
    }
    else if (msgType == NetMessageType::DestroyEntity)
    {
        uint32_t entityManagerNum;
        buf.readUint32(entityManagerNum);
        
        EntityManager* entityManager = getEntityManager(entityManagerNum);
        if (!entityManager)
        {
            return;
        }
        
        EntityId netEntityId;
        buf.readUint16(netEntityId);
        
        entityManager->freeGlobalEntity(netEntityId);
    }
}

void Client::handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType)
{
    if (msgType == NetMessageType::EntitySynchronize)
    {
        EntityManager* entityManager = getEntityManager(currentEntityManager);
        
        uint32_t numEntities;
        buf.readUint32(numEntities);
        
        for (uint32_t i = 0; i < numEntities; i++)
        {
            EntityId globalId;
            buf.readUint16(globalId);
            
            Entity* entity = entityManager->getGlobalEntity(globalId);
            if (!entity)
            {
                continue;
            }
            
            Entity::deserialize(*entity, buf);
        }
    }
}

void Client::handleEvents()
{
    eventHandler->refreshEvents();

    Event ev{};
    while (eventQueue->popEvent(ev))
    {
        if (consumeEvent(ev))
        {
            continue;
        }

        if (menuVisible && menu->consumeEvent(ev))
        {
            continue;
        }

        if (renderer->consumeEvent(ev))
        {
            continue;
        }
    }
}

bool Client::consumeEvent(const Event& ev)
{
    switch (ev.type)
    {
    case EventType::Quit:
        shutdown();
        return true;
    }

    return false;
}

void Client::tryRunTicks()
{
    if (clientState != ClientState::Connected)
    {
        return;
    }
    
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
}

void Client::draw()
{
    renderer->beginDraw();

    renderer->drawText("HELLO WORLD", glm::vec2{ 0.0f, 0.0f }, 100.0f);

    if (clientState == ClientState::Connected)
    {
        EntityManager* m = getEntityManager(currentEntityManager);
        auto entities = m->getGlobalEntities();
        for (auto entity : entities)
        {
            Entity* e = m->getGlobalEntity(entity);
            renderer->drawModel(*models[e->modelName], glm::vec3{1.0f}, e->rotation, e->position);
        }
    }

    if (menuVisible)
    {
        menu->draw();
    }

    renderer->endDraw();
}
