#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include <mutex>
#include "containers/memory.h"

struct sockaddr_in;

namespace platform
{
#ifdef _WIN32
    typedef uintptr_t SocketHandle;
#else
    typedef int SocketHandle;
#endif

    engine::string getSendErrorString(int errorCode);
    engine::string getConnectErrorString(int errorCode);

    enum class SocketType
    {
        TCP,
        UDP
    };

    class SocketServer;
    class Socket
    {
    public:
        engine::string ip() const;

        static engine::string localHostname();
        static engine::string localIp();

    private:
        friend class SocketServer;
        friend class SocketClient;
        friend class PrefixLengthMessageParser;

        uint32_t availableBytes() const;
        size_t read(char* buffer, size_t lengthBytes);
        size_t write(const char* buffer, size_t lengthBytes);

        static engine::shared_ptr<Socket> createSocket(int port, const engine::string& ip = "", SocketType type = SocketType::TCP);
        Socket(SocketHandle socket);
        SocketHandle m_socket;
        
        void enableSocketReuse();
        void enableNoDelay();
        void enableNonBlockingSocket();
        void enableBroadcasting();
        uint32_t dataAvailableInSocket();

        engine::shared_ptr<sockaddr_in> m_sockAddr;
        int connect();
        int bind();

        /*void private_read();*/
        size_t private_write();

        std::mutex m_mutex;
        bool m_nonBlocking;
        struct DataBlock
        {
            engine::vector<char> data;
            uint32_t currentPointer;
        };
        engine::vector<DataBlock> m_data;
    };
}
