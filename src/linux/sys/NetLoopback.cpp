#include "sys/NetLoopback.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdexcept>
#include <algorithm>
#include <string>
#include <random>

#include <fmt/format.h>

#include "Net.h"

NetLoopback::NetLoopback(bool initClient, bool initServer)
    : initClient{ initClient }, initServer{ initServer },
      serverSocket{ -1 }, clientSocket{ -1 }
{
    //calculate a random number for our client port
    {
        std::random_device dev;
        std::mt19937 rng{ dev() };
        std::uniform_int_distribution<std::mt19937::result_type> dist
        {
            1,
            std::numeric_limits<uint16_t>::max()
        };
        
        clientPort = static_cast<uint16_t>(dist(rng));
    }
    
    //figure out where to put our sockets
    {
        const char* rawRunDir = getenv("XDG_RUNTIME_DIR");
        if (!rawRunDir)
        {
            throw std::runtime_error{ "Failed to get environment variable XDG_RUNTIME_DIR" };
        }
        
        runDir = rawRunDir + std::string{"/tankgam"};
        
        //make the directory if it doesn't exist
        if (struct stat st = {};
        stat(runDir.c_str(), &st) == -1)
        {
            mkdir(runDir.c_str(), 0700);
        }
    }
    
    //initialize server socket
    if (initServer)
    {
        serverSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (serverSocket == -1)
        {
            throw std::runtime_error{ "Could not open server socket" };
        }
        
        const struct sockaddr_un serverAddr = getServerSockAddr();
        
        unlink(getServerName().c_str());
        if (bind(serverSocket, reinterpret_cast<const struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1)
        {
            throw std::runtime_error{ "bind failure serverSocket: make sure there's only one server instance running" };
        }
    }
    
    //initialize client socket
    if (initClient)
    {
        clientSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (clientSocket == -1)
        {
            throw std::runtime_error { "Could not open client socket" };
        }
        
        const struct sockaddr_un clientAddr = getClientSockAddr(clientPort);
        
        unlink(getClientName(clientPort).c_str());
        if (bind(clientSocket, reinterpret_cast<const struct sockaddr*>(&clientAddr), sizeof(clientAddr)) == -1)
        {
            throw std::runtime_error{ "bind failure clientSocket" };
        }
    }
}

NetLoopback::~NetLoopback()
{
    if (initClient)
    {
        close(clientSocket);
        
        unlink(getClientName(clientPort).c_str());
    }
    
    if (initServer)
    {
        close(serverSocket);
        
        unlink(getServerName().c_str());
    }
}

bool NetLoopback::getPacket(const NetSrc& src, NetBuf& buf, NetAddr& fromAddr)
{
    if (src == NetSrc::Server)
    {
        return getPacketServer(buf, fromAddr);
    }
    
    return getPacketClient(buf, fromAddr);
}

void NetLoopback::sendPacket(const NetSrc& src, NetBuf buf, const NetAddr& toAddr)
{
    if (src == NetSrc::Server)
    {
        sendPacketClient(std::move(buf), toAddr);
        return;
    }
    
    sendPacketServer(std::move(buf), toAddr);
}

std::string NetLoopback::getServerName()
{
    return runDir + "/server-socket";
}

std::string NetLoopback::getClientName(uint16_t port)
{
    return runDir + "/client-socket" + std::to_string(port);
}

uint16_t NetLoopback::getPortFromClientName(const std::string& clientName)
{
    const std::string port = clientName.substr((runDir + "/client-socket").size());
    
    return static_cast<uint16_t>(std::stoi(port));
}

struct sockaddr_un NetLoopback::getServerSockAddr()
{
    struct sockaddr_un serverAddr =
    {
        .sun_family = AF_UNIX,
    };
    strcpy(serverAddr.sun_path, getServerName().c_str());
    
    return serverAddr;
}

struct sockaddr_un NetLoopback::getClientSockAddr(uint16_t port)
{
    struct sockaddr_un clientAddr =
    {
        .sun_family = AF_UNIX,
    };
    strcpy(clientAddr.sun_path, getClientName(port).c_str());
    
    return clientAddr;
}

bool NetLoopback::getPacketServer(NetBuf& buf, NetAddr& fromAddr)
{
    struct sockaddr_un clientAddr{};
    socklen_t clientAddrLen = sizeof(struct sockaddr_un);
    
    struct pollfd pfds[1];
    pfds[0].fd = serverSocket;
    pfds[0].events = POLLIN;
    
    const int numEvents = poll(pfds, 1, 1);
    if (numEvents == 0)
    {
        return false;
    }
    
    std::array<std::byte, NetBuf::MAX_BYTES> data{};
    const ssize_t recvData = recvfrom(serverSocket, data.data(), data.size(), 0,
                                      reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
    if (recvData == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            throw std::runtime_error{ "Failure" };
        }
        
        return false;
    }
    
    buf.writeBytes(std::span<std::byte>{ data.data(), static_cast<size_t>(recvData) });
    
    fromAddr.type = NetAddrType::Loopback;
    fromAddr.port = getPortFromClientName(clientAddr.sun_path);
    
    return true;
}

bool NetLoopback::getPacketClient(NetBuf& buf, NetAddr& fromAddr)
{
    struct sockaddr_un serverAddr;
    socklen_t serverAddrLen = sizeof(struct sockaddr_un);
    
    struct pollfd pfds[1];
    pfds[0].fd = clientSocket;
    pfds[0].events = POLLIN;
    
    const int numEvents = poll(pfds, 1, 1);
    if (numEvents == 0)
    {
        return false;
    }
    
    std::array<std::byte, NetBuf::MAX_BYTES> data{};
    const ssize_t recvData = recvfrom(clientSocket, data.data(), data.size(), 0,
                                      reinterpret_cast<struct sockaddr*>(&serverAddr), &serverAddrLen);
    if (recvData == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            throw std::runtime_error{ "Failure" };
        }
        
        return false;
    }
    
    buf.writeBytes(std::span<std::byte>{ data.data(), static_cast<size_t>(recvData) });
    
    fromAddr.type = NetAddrType::Loopback;
    fromAddr.port = 0;
    
    return true;
}

void NetLoopback::sendPacketServer(NetBuf buf, const NetAddr& toAddr)
{
    const struct sockaddr_un serverAddr = getServerSockAddr();
    
    std::span<const std::byte> data = buf.getData();
    
    if (sendto(clientSocket, data.data(), data.size(), 0,
               reinterpret_cast<const struct sockaddr*>(&serverAddr), sizeof(struct sockaddr_un)) == -1)
    {
        throw std::runtime_error{ "Server sento err" };
    }
}

void NetLoopback::sendPacketClient(NetBuf buf, const NetAddr& toAddr)
{
    const struct sockaddr_un clientAddr = getClientSockAddr(toAddr.port);
    
    std::span<const std::byte> data = buf.getData();
    
    if (sendto(serverSocket, data.data(), data.size(), 0,
               reinterpret_cast<const struct sockaddr*>(&clientAddr), sizeof(struct sockaddr_un)) == -1)
    {
        throw std::runtime_error {fmt::format("Client sento err: {}", errno)};
    }
}