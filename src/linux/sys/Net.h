#pragma once

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <string>

#include "NetBuf.h"

enum class NetAddrType
{
    Unknown,
    Unix,
};

struct NetAddr
{
    NetAddrType type;
    uint16_t port;

    auto operator<=>(const NetAddr&) const = default;
};

enum class NetSrc
{
    Client,
    Server,
};

class Net
{
public:
    Net(bool initClient = true, bool initServer = true);
    ~Net();

    Net(const Net&) = delete;
    Net& operator=(const Net&) = delete;

    bool getPacket(NetSrc src, NetBuf& buf, NetAddr& fromAddr);
    void sendPacket(NetSrc src, NetBuf buf, NetAddr toAddr);

private:
    bool initClient;
    bool initServer;
    
    std::string runDir;
    
    uint16_t clientPort;
    
    int serverSocket;
    int clientSocket;
    
    std::string getServerName();
    std::string getClientName(uint16_t port);
    uint16_t getPortFromClientName(const std::string& clientName);
    
    struct sockaddr_un getServerSockAddr();
    struct sockaddr_un getClientSockAddr(uint16_t port);
    
    bool getPacketServer(NetBuf& buf, NetAddr& fromAddr);
    bool getPacketClient(NetBuf& buf, NetAddr& fromAddr);
    
    void sendPacketServer(NetBuf buf, NetAddr toAddr);
    void sendPacketClient(NetBuf buf, NetAddr toAddr);
};
