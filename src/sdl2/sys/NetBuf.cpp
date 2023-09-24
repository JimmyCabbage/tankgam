#include "sys/NetBuf.h"

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

void NetBuf::resetWrite()
{
    dataWritten = 0;
}

void NetBuf::resetRead()
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
        return writeByte(std::byte{ '\0' });
    }

    if (!writeBytes(std::span<const std::byte>
        {
        reinterpret_cast<const std::byte*>(str.data()),
        str.size()
        }))
    {
        return false;
    }

    //write a null terminator if there isn't one
    if (data[dataWritten - 1] != std::byte{ '\0' })
    {
        return writeByte(std::byte{ '\0' });
    }

    return true;
}

bool NetBuf::readString(std::string& str)
{
    str.clear();

    std::byte byte{ '\0' };
    do
    {
        if (!readByte(byte))
        {
            return false;
        }

        if (byte != std::byte{ '\0' })
        {
            str.push_back(static_cast<char>(byte));
        }
    } while (byte != std::byte{ '\0' });

    return true;
}

bool NetBuf::writeByte(std::byte byte)
{
    return writeBytes(std::span<std::byte>{ &byte, 1 });
}

bool NetBuf::readByte(std::byte& byte)
{
    if (!readBytes(std::span<std::byte>{ &byte, 1 }))
    {
        byte = std::numeric_limits<std::byte>::max();
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