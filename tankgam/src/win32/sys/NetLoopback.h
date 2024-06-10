#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "NetBuf.h"

struct NetAddr;
enum class NetSrc;
class Log;

struct NetLoopbackBuf
{
    std::array<NetBuf, 4> msgs;
    size_t recv; //number of msgs that we've read
    size_t send; //number of msgs that we've got
};

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

    NetLoopbackBuf clientLoopback;
    NetLoopbackBuf serverLoopback;
    
    NetLoopbackBuf& getLoopback(const NetSrc& src);
    NetLoopbackBuf& getOppositeLoopback(const NetSrc& src);
};
