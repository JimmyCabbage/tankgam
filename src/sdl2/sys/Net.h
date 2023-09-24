#pragma once

#include <array>
#include <span>
#include <cstdint>
#include <cstring>

#include "NetBuf.h"

enum class NetAddrType
{
    Unknown,
    Loopback,
};

struct NetAddr
{
    NetAddrType type;

    bool operator==(const NetAddr& o) const
    {
        if (type == NetAddrType::Loopback && o.type == NetAddrType::Loopback)
        {
            return true;
        }

        return false;
    }
};

enum class NetSrc
{
    Client,
    Server,
};

struct NetLoopback
{
    std::array<NetBuf, 4> msgs;
    size_t recv; //number of msgs that we've read
    size_t send; //number of msgs that we've got
};

class Net
{
public:
    Net();
    ~Net();

    Net(const Net&) = delete;
    Net& operator=(const Net&) = delete;

    bool getPacket(NetSrc src, NetBuf& buf, NetAddr& fromAddr);
    void sendPacket(NetSrc src, NetBuf buf, NetAddr toAddr);

private:
    NetLoopback clientLoopback;
    NetLoopback serverLoopback;

    NetLoopback& getLoopback(NetSrc src);
    NetLoopback& getOppositeLoopback(NetSrc src);
};
