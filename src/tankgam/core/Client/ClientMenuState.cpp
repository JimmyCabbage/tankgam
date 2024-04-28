#include "Client/ClientMenuState.h"

#include <memory>

#include "Client/ClientConnectingState.h"
#include "Client.h"
#include "Net.h"

ClientMenuState::ClientMenuState(Client& client, Renderer& renderer, Console& console, Net& net, MenuType menuType)
    : renderer{ renderer }, type{ menuType }
{
    switch (type)
    {
    case MenuType::MainMenu:
        {
            const auto mainListCallback = [this, &client, &renderer, &console, &net](size_t choice) -> void
            {
                switch (choice)
                {
                case 0:
                    client.pushState(std::make_shared<ClientConnectingState>(client, renderer, console, net,
                        NetAddr{ NetAddrType::Loopback, 0 }));
                    break;
                case 1:
                    client.shutdown();
                    break;
                }
            };

            MenuList mainList{ mainListCallback };
            mainList.addChoice("Start Game");
            mainList.addChoice("Quit");
            menu.addList(std::move(mainList));
        }
        break;
    }
}

ClientMenuState::~ClientMenuState() = default;

void ClientMenuState::pause()
{
}

void ClientMenuState::resume()
{
}

bool ClientMenuState::consumeEvent(const Event& ev)
{
    return menu.consumeEvent(ev);
}

void ClientMenuState::update()
{
}

void ClientMenuState::draw()
{
    menu.draw(renderer);
}

MenuType ClientMenuState::getMenuType() const
{
    return type;
}
