#pragma once

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <string>

#include "NetBuf.h"

struct NetAddr;
enum class NetSrc;

class NetLoopback
{
public:
    NetLoopback(bool initClient = true, bool initServer = true);
    ~NetLoopback();
    
    NetLoopback(const NetLoopback&) = delete;
    NetLoopback& operator=(const NetLoopback&) = delete;
    
    bool getPacket(const NetSrc& src, NetBuf& buf, NetAddr& fromAddr);
    void sendPacket(const NetSrc& src, NetBuf buf, const NetAddr& toAddr);
    
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
    
    void sendPacketServer(NetBuf buf, const NetAddr& toAddr);
    void sendPacketClient(NetBuf buf, const NetAddr& toAddr);
};
