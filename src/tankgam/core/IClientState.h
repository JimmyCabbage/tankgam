#pragma once

struct Event;
class Client;
class Renderer;

class IClientState
{
public:
    IClientState() = default;
    virtual ~IClientState() = default;

    // called when new state is made
    virtual void pause() = 0;

    // called when this state is restored
    virtual void resume() = 0;

    virtual bool consumeEvent(const Event& ev) = 0;
    virtual void update(Client& client, Renderer& renderer) = 0;
    virtual void draw(Renderer& renderer) = 0;

    virtual bool isFinished() = 0;
};
