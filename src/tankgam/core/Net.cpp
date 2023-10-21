#include "Net.h"

Net::Net(bool initClient, bool initServer)
    : netLoopback{ initClient, initServer }
{
}

Net::~Net() = default;

bool Net::getPacket(NetSrc src, NetBuf& buf, NetAddr& fromAddr)
{
    return netLoopback.getPacket(src, buf, fromAddr);
}

void Net::sendPacket(NetSrc src, NetBuf buf, NetAddr toAddr)
{
    netLoopback.sendPacket(src, std::move(buf), toAddr);
}
