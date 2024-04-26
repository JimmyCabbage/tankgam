#include "Client.h"

#include <stdexcept>
#include <numeric>

#include <fmt/format.h>

#include "sys/Console.h"
#include "sys/EventHandler.h"
#include "sys/Renderer.h"
#include "sys/Timer.h"
#include "Net.h"
#include "NetChan.h"
#include "NetBuf.h"
#include "Menu.h"
#include "Event.h"
#include "EntityManager.h"
#include "ClientMenuState.h"

#include <util/FileManager.h>

#include "IClientState.h"

Client::Client(Console& console, FileManager& fileManager, Net& net)
    : console{ console }, fileManager{ fileManager }, net{ net }
{
    try
    {
        console.log("Client: Init Event Subsystem...");
        eventQueue = std::make_unique<EventQueue>();
        eventHandler = std::make_unique<EventHandler>(*eventQueue);

        console.log("Client: Init Renderer Subsystem...");
        renderer = std::make_unique<Renderer>(console, fileManager, "src");

        pushState(std::make_unique<ClientMenuState>(*this, net, MenuType::MainMenu));
    }
    catch (const std::exception& e)
    {
        console.log(fmt::format("Client: Init Error:\n{}", e.what()));
        throw;
    }

    running = true;

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
        handleEvents();

        stateStack.top()->update(*this, *renderer);
        
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

/*void Client::showMenu()
{
    menuVisible = true;
}

void Client::hideMenu()
{
    menuVisible = false;
}*/

void Client::pushState(std::shared_ptr<IClientState> clientState)
{
    if (!stateStack.empty())
    {
        stateStack.top()->pause();
    }

    stateStack.push(std::move(clientState));
}

void Client::popState()
{
    stateStack.pop();

    if (!stateStack.empty())
    {
        stateStack.top()->resume();
    }
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

#if 0
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
        
        //prepare entity manager
        entityManager = std::make_unique<EntityManager>();
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
                models[entity->modelName] = renderer->createModel(entity->modelName);
            }
        }
        
        clientState = ClientState::Connected;
        
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
            models[newEntity->modelName] = renderer->createModel(newEntity->modelName);
        }
    }
    else if (msgType == NetMessageType::DestroyEntity)
    {
        EntityId netEntityId;
        buf.readUint16(netEntityId);
        
        entityManager->freeGlobalEntity(netEntityId);
    }
}

void Client::handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType)
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
                models[entity->modelName] = renderer->createModel(entity->modelName);
            }
        }
    }
}
#endif

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

        std::shared_ptr<IClientState> currentState = stateStack.top();
        if (currentState->consumeEvent(ev))
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
    default:
        return false;
    }
}

#if 0
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

void Client::sendPackets()
{
    if (clientState == ClientState::Disconnected)
    {
        return;
    }
    
    while (!commands.empty())
    {
        PlayerCommand& cmd = commands.front();
        commands.pop();
        
        NetBuf buf{};
        buf.writeFloat(cmd.addRotation);
        
        netChan->sendData(std::move(buf), NetMessageType::PlayerCommand);
    }
}
#endif

void Client::draw()
{
    renderer->beginDraw();

    renderer->drawText("HELLO WORLD", glm::vec2{ 0.0f, 0.0f }, 100.0f);

    std::shared_ptr<IClientState> currentState = stateStack.top();
    currentState->draw(*renderer);

    renderer->endDraw();
}
