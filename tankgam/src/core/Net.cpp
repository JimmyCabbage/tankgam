#include "Net.h"

#include <util/Log.h>

Net::Net(Log& log, bool initClient, bool initServer)
    : log{ log }, netLoopback{ log, initClient, initServer }
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
