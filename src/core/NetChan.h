/*
 * Inspired by gafferongames.com & quakeworld
 */

#pragma once

#include <cstddef>
#include <vector>
#include <string_view>
#include <array>
#include <numeric>

#include "sys/Net.h"
#include "NetBuf.h"

//bit 7 is set if it's a reliable message
enum class NetMessageType : uint8_t
{
    Unknown = 0,
    EntitySynchronize = 1,
    Synchronize = 1 | (1 << 7),
    CreateEntity = 2 | (1 << 7),
    DestroyEntity = 3 | (1 << 7),
    SendReliables = std::numeric_limits<uint8_t>::max(),
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

    void trySendReliable();

    //reliable
    void addReliableData(NetBuf sendBuf, NetMessageType msgType);

    //reliable
    void addReliableData(std::span<const std::byte> data, NetMessageType msgType);

    //unreliable
    void sendData(NetBuf sendBuf, NetMessageType msgType);

    //unreliable
    void sendData(std::span<const std::byte> data, NetMessageType msgType);

    bool processHeader(NetBuf& inBuf, NetMessageType& outType, std::vector<NetBuf>& outReliableMessages);

private:
    Net& net;
    NetSrc netSrc;
    NetAddr netAddr;

    static constexpr size_t PACKET_BUFFER_SIZE = 128;

    struct OutPacketInfo
    {
        bool acked;
        uint32_t sequence;
        NetBuf data;
    };

    struct InPacketInfo
    {
        bool acked;
    };

    struct OutHeader
    {
        NetMessageType msgType;
        uint32_t sequence;
        uint32_t ack;
        uint64_t ackBits;
        uint8_t numReliableMessages;
        std::vector<OutPacketInfo*> reliableMessages;
    };

    struct InHeader
    {
        NetMessageType msgType;
        uint32_t sequence;
        uint32_t ack;
        uint64_t ackBits;
        uint8_t numReliableMessages;
        std::vector<OutPacketInfo> reliableMessages;
    };

    void writeHeader(NetBuf& outBuf, NetMessageType msgType);

    bool readHeader(NetBuf& inBuf, InHeader& outHeader);

    //keeps track of if a packet has been recieved
    std::array<uint32_t, PACKET_BUFFER_SIZE> outgoingSequenceBuffer;

    //keeps track of if a packet has been acked
    std::array<OutPacketInfo, PACKET_BUFFER_SIZE> outgoingPacketInfoBuffer;

    //returns null if we don't have info about this packet
    OutPacketInfo* getOutPacketInfo(uint32_t sequence);

    //returns the packet info of this sequence
    OutPacketInfo& insertOutPacketInfo(uint32_t sequence);

    //keeps track of if a packet has been recieved
    std::array<uint32_t, PACKET_BUFFER_SIZE> incomingSequenceBuffer;

    //keeps track of if a packet has been acked
    std::array<InPacketInfo, PACKET_BUFFER_SIZE> incomingPacketInfoBuffer;

    //returns null if we don't have info about this packet
    InPacketInfo* getInPacketInfo(uint32_t sequence);

    //returns the packet info of this sequence
    InPacketInfo& insertInPacketInfo(uint32_t sequence);

    uint32_t outgoingSequence;
    uint32_t incomingSequence;

    uint32_t outgoingReliableSequence;
    uint32_t incomingReliableSequence;
    
    bool shouldTrySendReliable;
};
