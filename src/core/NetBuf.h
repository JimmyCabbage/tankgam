#pragma once

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <array>
#include <span>
#include <string_view>

class NetBuf
{
public:
    NetBuf();
    explicit NetBuf(std::span<const std::byte> newData);
    ~NetBuf();

    NetBuf(const NetBuf&) = delete;
    NetBuf& operator=(const NetBuf&) = delete;

    NetBuf(NetBuf&& o) noexcept;
    NetBuf& operator=(NetBuf&& o) noexcept;

    std::span<const std::byte> getData();

    void beginWrite();
    void beginRead();

    bool writeInt32(int32_t v);
    bool readInt32(int32_t& v);

    bool writeUint32(uint32_t v);
    bool readUint32(uint32_t& v);

    bool writeUint64(uint64_t v);
    bool readUint64(uint64_t& v);

    bool writeFloat(float v);
    bool readFloat(float& v);

    bool writeString(std::string_view str);
    bool readString(std::string& str);

    bool writeUint8(uint8_t byte);
    bool readUint8(uint8_t& byte);

    bool writeBytes(std::span<const std::byte> writeData);
    bool writeBytes(const std::byte* writeData, size_t writeSize);
    bool readBytes(std::span<std::byte> readData);

private:
    std::array<std::byte, 2048> data;
    size_t dataWritten;
    size_t dataRead;

    bool checkWriteSpaceLeft(size_t w);
    bool checkReadSpaceLeft(size_t r);
};
