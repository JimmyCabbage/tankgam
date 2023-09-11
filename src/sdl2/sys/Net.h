#pragma once

#include <array>
#include <span>
#include <cstdint>
#include <cstring>

enum class NetAddrType
{
    Loopback,
};

struct NetAddr
{
    NetAddrType type;
};

enum class NetSrc
{
    Client,
    Server,
};

class Net
{
public:
    Net();
    ~Net();

    Net(const Net&) = delete;
    Net& operator=(const Net&) = delete;

private:
};
