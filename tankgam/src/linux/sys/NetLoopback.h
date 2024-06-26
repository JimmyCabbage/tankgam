#pragma once

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <string>

#include "NetBuf.h"

struct NetAddr;
enum class NetSrc;
class Log;

class NetLoopback
{
public:
    NetLoopback(Log& log, bool initClient = true, bool initServer = true);
    ~NetLoopback();
    
    NetLoopback(const NetLoopback&) = delete;
    NetLoopback& operator=(const NetLoopback&) = delete;
    
    bool getPacket(const NetSrc& src, NetBuf& buf, NetAddr& fromAddr);
    bool sendPacket(const NetSrc& src, NetBuf buf, const NetAddr& toAddr);
    
private:
    Log& log;
    bool initClient;
    bool initServer;
    
    std::string runDir;
    
    uint16_t clientPort;
    
    int serverSocket;
    int clientSocket;
    
    uint16_t allocClientPort();
    void freeClientPort(uint16_t port);
    
    std::string getServerName();
    std::string getClientName(uint16_t port);
    uint16_t getPortFromClientName(const std::string& clientName);
    
    struct sockaddr_un getServerSockAddr();
    struct sockaddr_un getClientSockAddr(uint16_t port);
    
    bool getPacketServer(NetBuf& buf, NetAddr& fromAddr);
    bool getPacketClient(NetBuf& buf, NetAddr& fromAddr);
    
    bool sendPacketAsClient(NetBuf buf, const NetAddr& toAddr);
    bool sendPacketAsServer(NetBuf buf, const NetAddr& toAddr);
};
