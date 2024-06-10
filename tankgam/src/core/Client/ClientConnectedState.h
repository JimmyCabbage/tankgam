#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <queue>

#include "Client/IClientState.h"
#include "PlayerCommand.h"
#include "Net.h"

class Log;
class NetChan;
class NetBuf;
class Timer;
enum class NetMessageType : uint8_t;
class EntityManager;
class Model;
struct NetAddr;

class Renderer;

class ClientConnectedState : public IClientState
{
public:
    ClientConnectedState(Client& client, Renderer& renderer, Log& log,
        Net& net, NetAddr serverAddr,
        std::unique_ptr<NetChan> netChan, std::unique_ptr<Timer> timer,
        std::unique_ptr<EntityManager> entityManager,
        std::unordered_map<std::string, std::unique_ptr<Model>> models,
        uint32_t clientSalt, uint32_t serverSalt);
    ~ClientConnectedState() override;

    void pause() override;
    void resume() override;

    bool consumeEvent(const Event& ev) override;
    void update() override;

private:
    void handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr);
    void handleReliablePacket(NetBuf& buf, const NetMessageType& msgType);
    void handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType);

    void sendPackets();

public:
    void draw() override;

private:
    Client& client;
    Renderer& renderer;
    Log& log;

    Net& net;
    NetAddr serverAddr;
    std::unique_ptr<NetChan> netChan;
    std::unique_ptr<Timer> timer;

    std::unordered_map<std::string, std::unique_ptr<Model>> models;
    std::unique_ptr<EntityManager> entityManager;

    uint32_t clientSalt;
    uint32_t serverSalt;
    uint32_t combinedSalt;

    std::queue<PlayerCommand> commands;

    void disconnect(bool serverProbablyAlive);
};
