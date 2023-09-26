#pragma once

#include <cstddef>
#include <string_view>

#include "sys/Net.h"
#include "NetBuf.h"

enum class NetMessageType : unsigned char
{
    Unknown,
    Time
};

class NetChan
{
public:
    NetChan(Net& net, NetSrc netSrc);
    NetChan(Net& net, NetSrc netSrc, NetAddr toAddr);
    ~NetChan();

    NetChan(const NetChan&) = delete;
    NetChan& operator=(const NetChan&) = delete;

    void setToAddr(NetAddr toAddr);

    NetAddr getToAddr() const;

    //for sending a string without a direct connection
    void outOfBandPrint(NetAddr toAddr, std::string_view str);

    //for sending data without a connection
    void outOfBand(NetAddr toAddr, std::span<const std::byte> data);

    void sendData(NetBuf dataBuf, NetMessageType msgType);

    void sendData(std::span<const std::byte> data, NetMessageType msgType);

    NetMessageType processHeader(NetBuf& buf);

private:
    Net& net;
    NetSrc netSrc;
    NetAddr netAddr;

    uint32_t lastSequenceIn;
    uint32_t lastSequenceInAck;
    uint32_t lastSequenceOut;
    uint32_t lastSequenceOutAck;
};
