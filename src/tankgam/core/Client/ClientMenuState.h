#pragma once

#include "Client/IClientState.h"
#include "Client/Menu.h"

class Renderer;
class Client;
class Net;

enum class MenuType
{
    MainMenu,
};

class ClientMenuState : public IClientState
{
public:
    ClientMenuState(Client& client, Net& net, MenuType menuType);
    ~ClientMenuState() override;

    void pause() override;
    void resume() override;

    bool consumeEvent(const Event& ev) override;
    void update(Client& client, Renderer& renderer) override;
    void draw(Renderer& renderer) override;

    bool isFinished() override;

    MenuType getMenuType() const;

private:
    Menu menu;
    MenuType type;
};
