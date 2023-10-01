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
        if (!netChan->processHeader(buf, msgType, reliableMessages))
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

        if (msgType == NetMessageType::Unknown)
        {
            continue;
        }

        //TODO: handle unreliable messages here
    }

    //just to make sure reliable messages get sent (for now)
    netChan->sendData(std::span<const std::byte>{}, NetMessageType::Unknown);
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

        netChan->addReliableData(std::move(sendBuf), NetMessageType::Time);
    }
}

void Client::handleReliablePacket(NetBuf& buf, const NetMessageType& msgType)
{
    if (msgType == NetMessageType::Time)
    {
        uint64_t prevTime;
        buf.readUint64(prevTime);

        uint64_t serverTime;
        buf.readUint64(serverTime);

        const uint64_t roundTripTime = (timer->getTotalTicks() - prevTime);
        timer->setTickOffset(serverTime + (roundTripTime / 2));

        clientState = ClientState::Connected;
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
    if (clientState == ClientState::Connected)
    {
        const uint64_t ticks = timer->getPassedTicks();

        if (ticks == 0)
        {
            return;
        }

        for (uint64_t i = 0; i < ticks; i++)
        {
            //for (auto& [id, thing] : things)
            {
                //    thing->think();
            }
        }
    }
}

void Client::draw()
{
    renderer->beginDraw();

    renderer->drawText("HELLO WORLD ARE YOU THERE", glm::vec2{ 0.0f, 0.0f }, 100.0f);

    if (clientState == ClientState::Connected)
    {
        //if (playerId.has_value())
        {
            //things[playerId]
        }

        //for (auto& [id, thing] : things)
        {
            //    thing->draw();
        }
    }

    if (menuVisible)
    {
        menu->draw();
    }

    renderer->endDraw();
}
