#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "Client/IClientState.h"
#include "Net.h"

class Console;
class NetChan;
class NetBuf;
class Timer;
enum class NetMessageType : uint8_t;
class EntityManager;
class Model;

class Renderer;

class ClientConnectingState : public IClientState
{
public:
    ClientConnectingState(Client& client, Renderer& renderer, Console& console,
        Net& net, NetAddr serverAddr);
    ~ClientConnectingState() override;

    void pause() override;
    void resume() override;

    bool consumeEvent(const Event& ev) override;
    void update() override;

private:
    void handleUnconnectedPacket(NetBuf& buf, NetAddr& fromAddr);
    bool handleReliablePacket(NetBuf& buf, const NetMessageType& msgType);
    void handleUnreliablePacket(NetBuf& buf, const NetMessageType& msgType);

public:
    void draw() override;

private:
    Client& client;
    Renderer& renderer;
    Console& console;

    Net& net;
    NetAddr serverAddr;
    std::unique_ptr<NetChan> netChan;
    std::unique_ptr<Timer> timer;

    std::unordered_map<std::string, std::unique_ptr<Model>> models;
    std::unique_ptr<EntityManager> entityManager;

    uint64_t stopTryConnectTick;
    uint64_t nextSendTick;
    uint32_t clientSalt;
    uint32_t serverSalt;
    uint32_t combinedSalt;

    enum class ConnectState
    {
        Connecting,
        Challenging,
        AlmostConnected,
        Connected,
        GiveUp
    } connectState;

    NetBuf getSaltedBuffer();

    void trySendConnectionRequest();
    void trySendChallengeRequest();
    void trySendSynchronizeRequest();
};
