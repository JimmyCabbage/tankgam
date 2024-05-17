/*
 * Inspired by gafferongames.com & quakeworld
 */

#pragma once

#include <cstddef>
#include <vector>
#include <string_view>
#include <array>
#include <numeric>
#include <stdexcept>

#include "Net.h"
#include "NetBuf.h"

//bit 7 is set if it's a reliable message
enum class NetMessageType : uint8_t
{
    Unknown = 0,
    EntitySynchronize = 1,
    PlayerCommand = 2,
    Synchronize = 1 | (1 << 7),
    CreateEntity = 2 | (1 << 7),
    DestroyEntity = 3 | (1 << 7),
    SendReliables = std::numeric_limits<uint8_t>::max(),
};

constexpr const char* NetMessageTypeToString(const NetMessageType m)
{
    switch (m)
    {
    case NetMessageType::Unknown: return "Unknown";
    case NetMessageType::EntitySynchronize: return "EntitySynchronize";
    case NetMessageType::PlayerCommand: return "PlayerCommand";
    case NetMessageType::Synchronize: return "Synchronize";
    case NetMessageType::CreateEntity: return "CreateEntity";
    case NetMessageType::DestroyEntity: return "DestroyEntity";
    case NetMessageType::SendReliables: return "SendReliables";
    default: throw std::invalid_argument{ "UNKNOWN ENUM" };
    }
}

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

    static constexpr uint16_t OUT_OF_BAND_MAGIC_NUMBER = 15625;

    //for sending a string without a direct connection
    static void outOfBandPrint(Net& net, NetSrc src, NetAddr toAddr, std::string_view str);

    //for sending data without a connection
    static void outOfBand(Net& net, NetSrc src, NetAddr toAddr, NetBuf sendBuf);

    //for sending data without a connection
    static void outOfBand(Net& net, NetSrc src, NetAddr toAddr, std::span<const std::byte> data);

    static constexpr uint16_t RELIABLE_MAGIC_NUMBER = 3125;

    void trySendReliable(uint32_t salt);

    //reliable
    void addReliableData(NetBuf sendBuf, NetMessageType msgType);

    //reliable
    void addReliableData(std::span<const std::byte> data, NetMessageType msgType);

    //unreliable
    void sendData(NetBuf sendBuf, NetMessageType msgType, uint32_t salt);

    //unreliable
    void sendData(std::span<const std::byte> data, NetMessageType msgType, uint32_t salt);

    bool processHeader(NetBuf& inBuf, NetMessageType& outType, std::vector<NetBuf>& outReliableMessages, uint32_t expectedSalt);

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
        uint16_t magic;
        NetMessageType msgType;
        uint32_t salt;
        uint32_t sequence;
        uint32_t ack;
        uint64_t ackBits;
        uint8_t numReliableMessages;
        std::vector<OutPacketInfo*> reliableMessages;
    };

    struct InHeader
    {
        uint16_t magic;
        NetMessageType msgType;
        uint32_t salt;
        uint32_t sequence;
        uint32_t ack;
        uint64_t ackBits;
        uint8_t numReliableMessages;
        std::vector<OutPacketInfo> reliableMessages;
    };

    void writeHeader(NetBuf& outBuf, NetMessageType msgType, uint32_t salt);

    bool readHeader(NetBuf& inBuf, InHeader& outHeader, uint32_t expectedSalt);

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
