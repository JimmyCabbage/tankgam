#pragma once

#include "Client/IClientState.h"
#include "Client/Menu.h"

class Client;
class Renderer;
class Console;
class Net;

enum class MenuType
{
    MainMenu,
};

class ClientMenuState : public IClientState
{
public:
    ClientMenuState(Client& client, Renderer& renderer, Console& console, Net& net, MenuType menuType);
    ~ClientMenuState() override;

    void pause() override;
    void resume() override;

    bool consumeEvent(const Event& ev) override;
    void update() override;
    void draw() override;

    MenuType getMenuType() const;

private:
    Renderer& renderer;

    Menu menu;
    MenuType type;
};
