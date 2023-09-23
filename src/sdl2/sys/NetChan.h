#pragma once

#include <cstddef>
#include <string_view>

#include "sys/Net.h"
#include "sys/NetBuf.h"

class NetChan
{
public:
    NetChan(Net& net, NetSrc netSrc);
    ~NetChan();

    NetChan(const NetChan&) = delete;
    NetChan& operator=(const NetChan&) = delete;

    //for sending a string without a direct connection
    void outOfBandPrint(NetAddr toAddr, std::string_view str);

    //for sending data without a connection
    void outOfBand(NetAddr toAddr, std::span<const std::byte> data);

    void sendData(std::span<const std::byte> data);

    void processHeader(NetBuf& buf);

private:
    Net& net;
    NetSrc netSrc;

    uint32_t lastSequenceOut;
    uint32_t lastSequenceIn;
};
