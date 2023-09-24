#include "sys/NetChan.h"

#include <vector>

NetChan::NetChan(Net& net, NetSrc netSrc)
    : NetChan{ net, netSrc, NetAddr{ NetAddrType::Unknown } }
{
}

NetChan::NetChan(Net& net, NetSrc netSrc, NetAddr toAddr)
    : net{ net }, netSrc{ netSrc }, netAddr{ toAddr },
      lastSequenceIn{ 0 },
      lastSequenceOut{ 0 }, lastSequenceOutAck{ 0 }
{
}

NetChan::~NetChan() = default;

void NetChan::setToAddr(NetAddr toAddr)
{
    netAddr = toAddr;
}

NetAddr NetChan::getToAddr() const
{
    return netAddr;
}

void NetChan::outOfBandPrint(NetAddr toAddr, std::string_view str)
{
    std::span<const std::byte> strByte{ reinterpret_cast<const std::byte*>(str.data()), str.size() };

    std::vector<std::byte> data;
    data.resize(strByte.size());
    std::copy(strByte.begin(), strByte.end(), data.begin());
    data.push_back(std::byte{ '\0' });

    outOfBand(toAddr, data);
}

void NetChan::outOfBand(NetAddr toAddr, std::span<const std::byte> data)
{
    NetBuf buf{};
    buf.writeUint32(-1);
    buf.writeBytes(data);

    net.sendPacket(netSrc, std::move(buf), toAddr);
}

void NetChan::sendData(NetBuf sendBuf, NetMessageType msgType)
{
    sendData(sendBuf.getData(), msgType);
}

void NetChan::sendData(std::span<const std::byte> data, NetMessageType msgType)
{
    if (netAddr.type == NetAddrType::Unknown)
    {
        return;
    }

    NetBuf sendBuf{};
    sendBuf.writeUint32(lastSequenceIn);
    sendBuf.writeUint32(++lastSequenceOut);
    sendBuf.writeByte(static_cast<std::byte>(msgType));

    sendBuf.writeBytes(data);

    net.sendPacket(netSrc, std::move(sendBuf), netAddr);
}

NetMessageType NetChan::processHeader(NetBuf& buf)
{
    uint32_t packetSequenceIn; //the last packet it's seen, so our last out
    if (!buf.readUint32(packetSequenceIn))
    {
        return NetMessageType::Unknown;
    }

    uint32_t packetSequenceOut; //the sequence #, so our in
    if (!buf.readUint32(packetSequenceOut))
    {
        return NetMessageType::Unknown;
    }

    NetMessageType msgType = NetMessageType::Unknown;
    {
        std::byte netMsgType;
        if (!buf.readByte(netMsgType))
        {
            return NetMessageType::Unknown;
        }

        msgType = static_cast<NetMessageType>(netMsgType);
    }

    //duplicated or old packet
    if (packetSequenceOut <= lastSequenceIn)
    {
        return NetMessageType::Unknown;
    }

    //note that we have seen this new packet from them
    lastSequenceIn = packetSequenceOut;

    //a new packet has been ack'd
    if (packetSequenceIn > lastSequenceOutAck)
    {
        lastSequenceOutAck = packetSequenceIn;
    }

    return msgType;
}

