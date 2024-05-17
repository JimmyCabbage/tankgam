#include "sys/NetLoopback.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdexcept>
#include <algorithm>
#include <string>

#include <fmt/format.h>

#include "Net.h"

NetLoopback::NetLoopback(bool initClient, bool initServer)
    : initClient{ initClient }, initServer{ initServer },
      clientPort{ 0 },
      serverSocket{ -1 }, clientSocket{ -1 }
{
    //figure out where to put our sockets
    {
        const char* rawRunDir = getenv("XDG_RUNTIME_DIR");
        if (!rawRunDir)
        {
            rawRunDir = "/tmp";
        }
        
        runDir = rawRunDir + std::string{ "/tankgam" };
        
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
        
        //use shared memory to allocate a port
        clientPort = allocClientPort();
        
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
        
        freeClientPort(clientPort);
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

bool NetLoopback::sendPacket(const NetSrc& src, NetBuf buf, const NetAddr& toAddr)
{
    if (src == NetSrc::Server)
    {
        return sendPacketAsServer(std::move(buf), toAddr);
    }
    
    return sendPacketAsClient(std::move(buf), toAddr);
}

class ClientPortAllocator
{
public:
    ClientPortAllocator()
    {
        bool isNewAlloc = false;
        
        //first try creating the shared memory
        shm = shm_open("/tankgam-client-count-shm", O_RDWR | O_CREAT | O_EXCL, 0600);
        if (shm != -1)
        {
            //allocate memory
            if (ftruncate(shm, sizeof(int) * NUM_PORTS) == -1)
            {
                throw std::runtime_error{ "Could not use ftruncate on client count shared memory" };
            }
            
            isNewAlloc = true;
        }
        else
        {
            shm = shm_open("/tankgam-client-count-shm", O_RDWR | O_CREAT, 0600);
            if (shm == -1)
            {
                throw std::runtime_error{ fmt::format("Could not open client count shared memory: {}", strerror(errno))  };
            }
        }
        
        if (flock(shm, LOCK_EX) == -1)
        {
            throw std::runtime_error{ "Could not lock client count shared memory" };
        }
        
        portArray = static_cast<int*>(mmap(nullptr, PORT_ARRAY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0));
        if (portArray == MAP_FAILED)
        {
            throw std::runtime_error{ "Could not mmap client count shared memory" };
        }
        
        if (isNewAlloc)
        {
            memset(portArray, 0, PORT_ARRAY_SIZE);
        }
    }
    
    ~ClientPortAllocator()
    {
        if (munmap(portArray, PORT_ARRAY_SIZE) == -1)
        {
            throw std::runtime_error{ "Could not unmap client count shared memory" };
        }
        
        if (flock(shm, LOCK_UN) == -1)
        {
            throw std::runtime_error{ "Could not unlock client count shared memory" };
        }
        
        if (close(shm) == -1)
        {
            throw std::runtime_error{ "Could not close client count shared memory" };
        }
    }
    
    ClientPortAllocator(const ClientPortAllocator&) = delete;
    ClientPortAllocator& operator=(const ClientPortAllocator&) = delete;
    
    std::span<int> getPorts()
    {
        return std::span<int>{ portArray, NUM_PORTS };
    }

private:
    int shm;
    
    static constexpr size_t NUM_PORTS = 64;
    static constexpr size_t PORT_ARRAY_SIZE = sizeof(int) * NUM_PORTS;
    int* portArray;
};

uint16_t NetLoopback::allocClientPort()
{
    ClientPortAllocator portAlloc{};
    const auto ports = portAlloc.getPorts();
    
    uint16_t port = 0;
    
    bool foundPort = false;
    for (size_t i = 0; i < ports.size(); i++)
    {
        if (ports[i] == 0)
        {
            ports[i] = 1;
            port = i;
            
            foundPort = true;
            break;
        }
    }
    
    if (!foundPort)
    {
        throw std::runtime_error{ "Could not find available client port" };
    }
    
    return port;
}

void NetLoopback::freeClientPort(uint16_t port)
{
    ClientPortAllocator portAlloc{};
    const auto ports = portAlloc.getPorts();
    
    ports[port] = 0;
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
    
    strncpy(clientAddr.sun_path, getClientName(port).c_str(), sizeof clientAddr.sun_path - 1);
    
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
            throw std::runtime_error{ fmt::format("Failure: {}", strerror(errno)) };
        }
        
        return false;
    }
    
    buf.writeBytes(std::span<std::byte>{ data.data(), static_cast<size_t>(recvData) });
    
    fromAddr.type = NetAddrType::Loopback;
    fromAddr.port = 0;
    
    return true;
}

bool NetLoopback::sendPacketAsClient(NetBuf buf, const NetAddr& toAddr)
{
    const struct sockaddr_un serverAddr = getServerSockAddr();
    
    std::span<const std::byte> data = buf.getData();

    const ssize_t res = sendto(clientSocket, data.data(), data.size(), 0,
        reinterpret_cast<const struct sockaddr*>(&serverAddr), sizeof(struct sockaddr_un));
    if (res == -1)
    {
        if (errno == ENOENT)
        {
            return false;
        }

        throw std::runtime_error{ fmt::format("Server sento err: {}({})", strerror(errno), errno) };
    }

    return true;
}

bool NetLoopback::sendPacketAsServer(NetBuf buf, const NetAddr& toAddr)
{
    const struct sockaddr_un clientAddr = getClientSockAddr(toAddr.port);

    std::span<const std::byte> data = buf.getData();
    
    const ssize_t res = sendto(serverSocket, data.data(), data.size(), 0,
        reinterpret_cast<const struct sockaddr*>(&clientAddr), sizeof(struct sockaddr_un));

    if (res == -1)
    {
        if (errno == ENOENT)
        {
            return false;
        }

        throw std::runtime_error{ fmt::format("Client sento err: {}({})", strerror(errno), errno) };
    }

    return true;
}
