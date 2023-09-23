#include "sys/NetChan.h"

#include <vector>

NetChan::NetChan(Net& net, NetSrc netSrc)
    : net{ net }, netSrc{ netSrc }, lastSequenceIn{ 0 }, lastSequenceOut{ 0 }
{
}

NetChan::~NetChan() = default;

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

void NetChan::sendData(std::span<const std::byte> data)
{

}

void NetChan::processHeader(NetBuf& buf)
{
    
}

