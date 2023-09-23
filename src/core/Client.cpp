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
#include "sys/NetChan.h"
#include "Menu.h"
#include "Event.h"

Client::Client(Console& console, Net& net)
    : console{ console }, net{ net }
{
    try
    {
        netChan = std::make_unique<NetChan>(net, NetSrc::Client);

        console.log("Client: Init File Subsystem...");
        fileManager = std::make_unique<FileManager>(console);
        fileManager->loadAssetsFile("dev.assets");
        fileManager->loadAssetsFile("tank.assets");

        console.log("Client: Init Event Subsystem...");
        eventQueue = std::make_unique<EventQueue>();
        eventHandler = std::make_unique<EventHandler>(*eventQueue);

        console.log("Client: Init Renderer Subsystem...");
        renderer = std::make_unique<Renderer>(console, *fileManager, "src");

        console.log("Client: Init Timer Subsystem...");
        timer = std::make_unique<Timer>();

        lastTick = 0;

        console.log("Client: Init Menu Subsystem...");
        menu = std::make_unique<Menu>(*renderer);

        const auto mainListCallback = [this](size_t choice) -> void
        {
            switch (choice)
            {
            case 0:
                //TODO: add connection details here
                //changeState(ClientState::Game);
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

    clientState = ClientState::Disconnected;

    running = true;

    menuVisible = true;

    console.log("Client: Initialized");

    netChan->outOfBandPrint(NetAddr{NetAddrType::Loopback}, "connect");
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

        tryRunTicks();

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

    timer->pause();
    console.log("Client: Showing Menu");
}

void Client::hideMenu()
{
    menuVisible = false;

    timer->unpause();
    console.log("Client: Hiding Menu");
}

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
        break;
    }
}

void Client::tryRunTicks()
{
    if (menuVisible)
    {
        return;
    }

    if (clientState == ClientState::Connected)
    {
        const uint64_t currTicks = timer->getPassedTicks();
        const uint64_t ticks = currTicks - lastTick;
        lastTick = currTicks;

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

    const float rotation = -static_cast<float>(lastTick % 360);

    renderer->endDraw();
}
