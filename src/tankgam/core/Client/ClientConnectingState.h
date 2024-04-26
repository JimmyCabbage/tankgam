#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "Client/IClientState.h"

class Net;
class NetChan;
class NetBuf;
class Timer;
enum class NetMessageType : uint8_t;
class EntityManager;
class Model;
struct NetAddr;

class Renderer;

class ClientConnectingState : public IClientState
{
public:
    ClientConnectingState(Net& net, NetAddr serverAddr);
    ~ClientConnectingState() override;

    void pause() override;
    void resume() override;

    bool consumeEvent(const Event& ev) override;
    void update(Client& client, Renderer& renderer) override;

private:
    void handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr);
    void handleReliablePacket(Client& client, Renderer& renderer, NetBuf& buf, const NetMessageType& msgType);
    void handleUnreliablePacket(Client& client, Renderer& renderer, NetBuf& buf, const NetMessageType& msgType);

public:
    void draw(Renderer& renderer) override;

    bool isFinished() override;

private:
    Net& net;
    std::unique_ptr<NetChan> netChan;
    std::unique_ptr<Timer> timer;

    std::unordered_map<std::string, std::unique_ptr<Model>> models;
    std::unique_ptr<EntityManager> entityManager;

    size_t stopTryConnectTick;
    bool connected;
};
