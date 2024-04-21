#pragma once

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <string>

#include "NetBuf.h"
#include "sys/NetLoopback.h"

enum class NetAddrType
{
    Unknown,
    Loopback,
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
    NetLoopback netLoopback;
};
