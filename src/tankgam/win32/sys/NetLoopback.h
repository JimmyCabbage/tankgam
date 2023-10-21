#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "NetBuf.h"

struct NetAddr;
enum class NetSrc;

struct NetLoopbackBuf
{
    std::array<NetBuf, 4> msgs;
    size_t recv; //number of msgs that we've read
    size_t send; //number of msgs that we've got
};

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
    NetLoopbackBuf clientLoopback;
    NetLoopbackBuf serverLoopback;
    
    NetLoopbackBuf& getLoopback(const NetSrc& src);
    NetLoopbackBuf& getOppositeLoopback(const NetSrc& src);
};
