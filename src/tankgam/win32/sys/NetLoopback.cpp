#include "sys/NetLoopback.h"

#include <stdexcept>
#include <algorithm>

#include "Net.h"

NetLoopback::NetLoopback(bool /* initClient */, bool /* initServer */)
    : clientLoopback{ {}, 0, 0 }, serverLoopback{ {}, 0, 0 }
{
}

NetLoopback::~NetLoopback() = default;

bool NetLoopback::getPacket(const NetSrc& src, NetBuf& buf, NetAddr& fromAddr)
{
    NetLoopbackBuf& loop = getLoopback(src);
    
    //if more messages were sent than could be stored in the loopback buffer
    if (loop.send - loop.recv > loop.msgs.size())
    {
        //limit to the most recent of the loopback buffer
        loop.recv = loop.send - loop.msgs.size();
    }
    
    //nothing sent we haven't checked
    if (loop.recv >= loop.send)
    {
        return false;
    }
    
    //get the message's index
    //also increase the number of msgs we've read
    const size_t i = loop.recv++ % loop.msgs.size();
    
    buf = std::move(loop.msgs[i]);
    fromAddr.type = NetAddrType::Loopback;
    
    return true;
}

bool NetLoopback::sendPacket(const NetSrc& src, NetBuf buf, const NetAddr& /*toAddr*/)
{
    NetLoopbackBuf& loop = getOppositeLoopback(src);
    
    //get the next available message's index
    //also make the index increment
    const size_t i = loop.send++ % loop.msgs.size();
    
    loop.msgs[i] = std::move(buf);

    return true;
}

NetLoopbackBuf& NetLoopback::getLoopback(const NetSrc& src)
{
    if (src == NetSrc::Client)
    {
        return clientLoopback;
    }
    else if (src == NetSrc::Server)
    {
        return serverLoopback;
    }
    
    throw std::runtime_error{ "getLoopback: Unknown NetSrc type" };
}

NetLoopbackBuf& NetLoopback::getOppositeLoopback(const NetSrc& src)
{
    if (src == NetSrc::Client)
    {
        return serverLoopback;
    }
    else if (src == NetSrc::Server)
    {
        return clientLoopback;
    }
    
    throw std::runtime_error{ "getOppositeLoopback: Unknown NetSrc type" };
}
