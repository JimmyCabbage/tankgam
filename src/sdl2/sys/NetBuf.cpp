#include "sys/NetBuf.h"

#include <stdexcept>
#include <algorithm>

#include <fmt/format.h>

NetBuf::NetBuf()
    : data{}, dataWritten{ 0 }, dataRead{ 0 }
{
}

NetBuf::NetBuf(std::span<const unsigned char> newData)
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

std::span<const unsigned char> NetBuf::getData()
{
    return std::span<const unsigned char>
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
    std::span<unsigned char> byteArray{ reinterpret_cast<unsigned char*>(&v), sizeof(v) };

    return writeBytes(byteArray);
}

bool NetBuf::readInt32(int32_t& v)
{
    std::span<unsigned char> byteArray{ reinterpret_cast<unsigned char*>(&v), sizeof(v) };

    if (!readBytes(byteArray))
    {
        v = -1;
        return false;
    }

    return true;
}

bool NetBuf::writeFloat(float v)
{
    std::span<unsigned char> byteArray{ reinterpret_cast<unsigned char*>(&v), sizeof(v) };

    return writeBytes(byteArray);
}

bool NetBuf::readFloat(float& v)
{
    std::span<unsigned char> byteArray{ reinterpret_cast<unsigned char*>(&v), sizeof(v) };

    if (!readBytes(byteArray))
    {
        v = -1;
        return false;
    }

    return true;
}

bool NetBuf::writeString(std::string_view str)
{
    if (!writeBytes(std::span<const unsigned char>
        {
        reinterpret_cast<const unsigned char*>(str.data()),
        str.size()
        }))
    {
        return false;
    }

    //write a null terminator just in case this string_view doesn't have a null terminator at the end
    return writeByte('\0');
}

bool NetBuf::readString(std::string& str)
{
    str.clear();

    unsigned char byte = '\0';
    do
    {
        if (!readByte(byte))
        {
            return false;
        }

        str.push_back(static_cast<char>(byte));
    } while (byte != '\0');

    return true;
}

bool NetBuf::writeByte(unsigned char byte)
{
    return writeBytes(std::span<unsigned char>{ &byte, 1 });
}

bool NetBuf::readByte(unsigned char& byte)
{
    if (!readBytes(std::span<unsigned char>{ &byte, 1 }))
    {
        byte = -1;
        return false;
    }

    return true;
}

bool NetBuf::writeBytes(std::span<const unsigned char> writeData)
{
    if (!checkWriteSpaceLeft(writeData.size()))
    {
        return false;
    }

    std::copy(writeData.begin(), writeData.end(), data.begin() + dataWritten);

    dataWritten += writeData.size();

    return true;
}

bool NetBuf::readBytes(std::span<unsigned char> readData)
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