#pragma once

#include <cstdint>
#include <cstring>
#include <array>
#include <span>
#include <string_view>

class NetBuf
{
public:
    NetBuf();
    explicit NetBuf(std::span<const unsigned char> newData);
    ~NetBuf();

    NetBuf(const NetBuf&) = delete;
    NetBuf& operator=(const NetBuf&) = delete;

    NetBuf(NetBuf&& o) noexcept;
    NetBuf& operator=(NetBuf&& o) noexcept;

    std::span<const unsigned char> getData();

    void resetWrite();
    void resetRead();

    bool writeInt32(int32_t v);
    bool readInt32(int32_t& v);

    bool writeFloat(float v);
    bool readFloat(float& v);

    bool writeString(std::string_view str);
    bool readString(std::string& str);

    bool writeByte(unsigned char byte);
    bool readByte(unsigned char& byte);

    bool writeBytes(std::span<const unsigned char> writeData);
    bool readBytes(std::span<unsigned char> readData);

private:
    std::array<unsigned char, 1024> data;
    size_t dataWritten;
    size_t dataRead;

    bool checkWriteSpaceLeft(size_t w);
    bool checkReadSpaceLeft(size_t r);
};
