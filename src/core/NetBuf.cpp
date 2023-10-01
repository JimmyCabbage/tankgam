#include "NetBuf.h"

#include <stdexcept>
#include <algorithm>
#include <limits>

#include <fmt/format.h>

NetBuf::NetBuf()
    : data{}, dataWritten{ 0 }, dataRead{ 0 }
{
}

NetBuf::NetBuf(std::span<const std::byte> newData)
{
    if (newData.size() > data.size())
    {
        throw std::runtime_error{ "Tried to create network buffer greater than the available space" };
    }

    std::copy(newData.begin(), newData.end(), data.begin());
    dataWritten = newData.size();
    dataRead = 0;
}

NetBuf::~NetBuf() = default;

NetBuf::NetBuf(NetBuf&& o) noexcept
{
    std::swap(data, o.data);
    std::swap(dataWritten, o.dataWritten);
    std::swap(dataRead, o.dataRead);
}

NetBuf& NetBuf::operator=(NetBuf&& o) noexcept
{
    if (&o == this)
    {
        return *this;
    }

    std::swap(data, o.data);
    std::swap(dataWritten, o.dataWritten);
    std::swap(dataRead, o.dataRead);

    return *this;
}

std::span<const std::byte> NetBuf::getData()
{
    return std::span<const std::byte>
    {
        data.begin(), data.begin() + dataWritten
    };
}

void NetBuf::beginWrite()
{
    dataWritten = 0;
}

void NetBuf::beginRead()
{
    dataRead = 0;
}

//TODO: handle endianness

bool NetBuf::writeInt32(int32_t v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    return writeBytes(byteArray);
}

bool NetBuf::readInt32(int32_t& v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    if (!readBytes(byteArray))
    {
        v = -1;
        return false;
    }

    return true;
}

bool NetBuf::writeUint32(uint32_t v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    return writeBytes(byteArray);
}

bool NetBuf::readUint32(uint32_t& v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    if (!readBytes(byteArray))
    {
        v = -1;
        return false;
    }

    return true;
}

bool NetBuf::writeUint64(uint64_t v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    return writeBytes(byteArray);
}

bool NetBuf::readUint64(uint64_t& v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    if (!readBytes(byteArray))
    {
        v = -1;
        return false;
    }

    return true;
}

bool NetBuf::writeFloat(float v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    return writeBytes(byteArray);
}

bool NetBuf::readFloat(float& v)
{
    std::span<std::byte> byteArray{ reinterpret_cast<std::byte*>(&v), sizeof(v) };

    if (!readBytes(byteArray))
    {
        v = -1;
        return false;
    }

    return true;
}

bool NetBuf::writeString(std::string_view str)
{
    if (str.empty())
    {
        return writeUint8('\0');
    }

    for (const auto c : str)
    {
        if (c == '\0')
        {
            break;
        }

        if (!writeUint8(c))
        {
            return false;
        }
    }

    //write out a null terminator
    return writeUint8('\0');
}

bool NetBuf::readString(std::string& str)
{
    str.clear();

    uint8_t byte{ '\0' };
    do
    {
        if (!readUint8(byte))
        {
            return false;
        }

        if (byte != '\0')
        {
            str.push_back(static_cast<char>(byte));
        }
    } while (byte != '\0');

    return true;
}

bool NetBuf::writeUint8(std::uint8_t byte)
{
    return writeBytes(std::span<std::byte>{ reinterpret_cast<std::byte*>(&byte), sizeof(byte) });
}

bool NetBuf::readUint8(std::uint8_t& byte)
{
    if (!readBytes(std::span<std::byte>{ reinterpret_cast<std::byte*>(&byte), 1 }))
    {
        byte = std::numeric_limits<uint8_t>::max();
        return false;
    }

    return true;
}

bool NetBuf::writeBytes(std::span<const std::byte> writeData)
{
    if (!checkWriteSpaceLeft(writeData.size()))
    {
        return false;
    }

    std::copy(writeData.begin(), writeData.end(), data.begin() + dataWritten);

    dataWritten += writeData.size();

    return true;
}

bool NetBuf::writeBytes(const std::byte* writeData, size_t writeSize)
{
    writeBytes(std::span{ writeData, writeSize });
}

bool NetBuf::readBytes(std::span<std::byte> readData)
{
    if (!checkReadSpaceLeft(readData.size()))
    {
        return false;
    }

    std::copy(data.begin() + dataRead, data.begin() + dataRead + readData.size(), readData.data());

    dataRead += readData.size();

    return true;
}

bool NetBuf::checkWriteSpaceLeft(size_t w)
{
    if (dataWritten + w > data.size())
    {
        return false;
    }

    return true;
}

bool NetBuf::checkReadSpaceLeft(size_t r)
{
    if (dataRead + r > dataWritten)
    {
        return false;
    }

    return true;
}