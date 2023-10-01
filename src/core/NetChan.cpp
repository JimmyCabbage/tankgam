#include "NetChan.h"

#include <vector>
#include <stdexcept>

NetChan::NetChan(Net& net, NetSrc netSrc)
    : NetChan{ net, netSrc, NetAddr{ NetAddrType::Unknown } }
{
}

NetChan::NetChan(Net& net, NetSrc netSrc, NetAddr toAddr)
    : net{ net }, netSrc{ netSrc }, netAddr{ toAddr },
      outgoingSequenceBuffer{}, outgoingPacketInfoBuffer{},
      outgoingSequence{ 0 }, incomingSequence{ 0 },
      outgoingReliableSequence{ 0 }, incomingReliableSequence{ 0 },
      trySendReliableCounter{0 }
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

void NetChan::trySendReliable()
{
    constexpr size_t RETRY_FREQUENCY = 6;
    trySendReliableCounter = (trySendReliableCounter + 1) % RETRY_FREQUENCY;

    if (trySendReliableCounter == 0)
    {
        sendData(std::span<const std::byte>{}, NetMessageType::SendReliables);
    }
}

void NetChan::addReliableData(NetBuf sendBuf, NetMessageType msgType)
{
    addReliableData(sendBuf.getData(), msgType);
}

void NetChan::addReliableData(std::span<const std::byte> data, NetMessageType msgType)
{
    if (netAddr.type == NetAddrType::Unknown)
    {
        return;
    }
    //check if this is supposed to be a reliable message
    else if (!(static_cast<uint8_t>(msgType) & 1 << 7))
    {
        throw std::runtime_error{ "Tried to send unreliable message through reliable messaging" };
    }

    NetBuf msgBuf{};
    msgBuf.writeUint8(static_cast<uint8_t>(msgType));
    msgBuf.writeBytes(data);

    OutPacketInfo& newPacket = insertOutPacketInfo(++outgoingReliableSequence);
    newPacket.acked = false;
    newPacket.sequence = outgoingReliableSequence;
    newPacket.data = std::move(msgBuf);
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
    //check if this is supposed to be a reliable message
    else if (static_cast<uint8_t>(msgType) & 1 << 7)
    {
        if (msgType != NetMessageType::SendReliables) //exception
        {
            throw std::runtime_error{"Tried to send reliable message through unreliable messaging"};
        }
    }

    NetBuf sendBuf{};
    writeHeader(sendBuf, msgType);

    sendBuf.writeBytes(data);

    net.sendPacket(netSrc, std::move(sendBuf), netAddr);
}

bool NetChan::processHeader(NetBuf& inBuf, NetMessageType& outType, std::vector<NetBuf>& outReliableMessages)
{
    InHeader header{};
    if (!readHeader(inBuf, header))
    {
        return false;
    }

    if (header.sequence < incomingSequence)
    {
        return false;
    }

    incomingSequence = header.sequence;

    outType = header.msgType;
    outReliableMessages.clear();

    //mark down this new acked packet we put out
    if (OutPacketInfo* packetInfo = getOutPacketInfo(header.ack);
        packetInfo && !packetInfo->acked)
    {
        packetInfo->acked = true;
    }

    //mark down the other acks that they made
    for (uint32_t counter = 0, currentSequence = header.ack - 1;
         counter < 64;
         counter++, currentSequence--)
    {
        OutPacketInfo* packetInfo = getOutPacketInfo(currentSequence);
        if (!packetInfo || packetInfo->acked)
        {
            continue;
        }

        if (header.ackBits & (1 << counter))
        {
            packetInfo->acked = true;
        }
    }

    //technically this is THEIR out reliable info that we're reading
    for (OutPacketInfo& packetInfo : header.reliableMessages)
    {
        //check if we already got this
        if (getInPacketInfo(packetInfo.sequence)) //if it exists we got it
        {
            continue;
        }

        //create a listing for this
        InPacketInfo& inPacketInfo = insertInPacketInfo(packetInfo.sequence);

        //ack that we got this
        inPacketInfo.acked = true;

        //write this out to the caller
        outReliableMessages.push_back(std::move(packetInfo.data));
    }

    return true;
}

void NetChan::writeHeader(NetBuf& outBuf, NetMessageType msgType)
{
    //for organizational purposes
    OutHeader header
    {
        .msgType = msgType,
        .sequence = ++outgoingSequence,
        .ack = incomingReliableSequence,
        .ackBits = 0,
        .reliableMessages = {},
    };

    //figure out the reliable ack
    for (uint32_t counter = 0, currentSequence = incomingReliableSequence - 1;
         counter < 64;
         counter++, currentSequence--)
    {
        InPacketInfo* packetInfo = getInPacketInfo(currentSequence);
        if (!packetInfo)
        {
            continue;
        }

        if (packetInfo->acked)
        {
            header.ackBits |= 1 << counter;
        }
    }

    std::vector<OutPacketInfo*> reliableMessages;

    if (OutPacketInfo* packetInfo = getOutPacketInfo(outgoingReliableSequence); packetInfo && !packetInfo->acked)
    {
        reliableMessages.push_back(packetInfo);
    }

    //figure out the reliable messages we need to send
    for (uint32_t counter = 0, currentSequence = outgoingReliableSequence - 1;
         counter < 64;
         counter++, currentSequence--)
    {
        OutPacketInfo* packetInfo = getOutPacketInfo(currentSequence);
        if (!packetInfo || packetInfo->acked)
        {
            continue;
        }

        reliableMessages.push_back(packetInfo);
    }

    header.numReliableMessages = static_cast<uint8_t>(reliableMessages.size());
    header.reliableMessages = std::move(reliableMessages);

    outBuf.writeUint8(static_cast<uint8_t>(header.msgType));
    outBuf.writeUint32(header.sequence);
    outBuf.writeUint32(header.ack);
    outBuf.writeUint64(header.ackBits);
    outBuf.writeUint8(header.numReliableMessages);
    for (OutPacketInfo* reliableMessage : header.reliableMessages)
    {
        outBuf.writeUint32(reliableMessage->sequence);
        outBuf.writeUint32(reliableMessage->data.getData().size());
        outBuf.writeBytes(reliableMessage->data.getData());
    }
}

bool NetChan::readHeader(NetBuf& inBuf, InHeader& outHeader)
{
    {
        uint8_t tempV;
        if (!inBuf.readUint8(tempV))
        {
            return false;
        }

        outHeader.msgType = static_cast<NetMessageType>(tempV);
    }

    if (!inBuf.readUint32(outHeader.sequence))          return false;
    if (!inBuf.readUint32(outHeader.ack))               return false;
    if (!inBuf.readUint64(outHeader.ackBits))           return false;
    if (!inBuf.readUint8(outHeader.numReliableMessages))  return false;

    outHeader.reliableMessages.resize(static_cast<size_t>(outHeader.numReliableMessages));

    for (auto& reliableMessage : outHeader.reliableMessages)
    {
        if (!inBuf.readUint32(reliableMessage.sequence))
        {
            return false;
        }

        uint32_t dataSize;
        if (!inBuf.readUint32(dataSize))
        {
            return false;
        }

        std::vector<std::byte> data;
        data.resize(static_cast<size_t>(dataSize));
        if (!inBuf.readBytes(data))
        {
            return false;
        }

        if (!reliableMessage.data.writeBytes(data))
        {
            return false;
        }

        reliableMessage.acked = false; //technically superfluous
    }

    return true;
}

NetChan::OutPacketInfo* NetChan::getOutPacketInfo(uint32_t sequence)
{
    const size_t index = static_cast<size_t>(sequence) % PACKET_BUFFER_SIZE;
    if (outgoingSequenceBuffer[index] == sequence)
    {
        return &outgoingPacketInfoBuffer[index];
    }
    else
    {
        return nullptr;
    }
}

NetChan::OutPacketInfo& NetChan::insertOutPacketInfo(uint32_t sequence)
{
    const size_t index = static_cast<size_t>(sequence) % PACKET_BUFFER_SIZE;
    outgoingSequenceBuffer[index] = sequence;
    return outgoingPacketInfoBuffer[index];
}

NetChan::InPacketInfo* NetChan::getInPacketInfo(uint32_t sequence)
{
    const size_t index = static_cast<size_t>(sequence) % PACKET_BUFFER_SIZE;
    if (incomingSequenceBuffer[index] == sequence)
    {
        return &incomingPacketInfoBuffer[index];
    }
    else
    {
        return nullptr;
    }
}

NetChan::InPacketInfo& NetChan::insertInPacketInfo(uint32_t sequence)
{
    const size_t index = static_cast<size_t>(sequence) % PACKET_BUFFER_SIZE;
    incomingSequenceBuffer[index] = sequence;
    return incomingPacketInfoBuffer[index];
}
