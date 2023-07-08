#include "platform/network/Socket.h"
#include "platform/network/SocketErrorCodes.h"
#include "platform/network/SocketSystemInitialization.h"
#include "tools/Debug.h"
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <Winsock2.h>
#else
#ifdef OSX
#define Polygon CarbonPolygon
#include <Carbon/Carbon.h>
#undef Polygon
#endif

#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

namespace platform
{
    Socket::Socket(uintptr_t socket)
        : m_socket{ socket }
        , m_sockAddr{ engine::make_shared<sockaddr_in>() }
        , m_nonBlocking{ false }
    {}

    engine::string getReadErrorString(int errorCode)
    {
        switch (errorCode)
        {
        case WSANOTINITIALISED: { return engine::string("A successful WSAStartup call must occur before using this function."); }
        case WSAENETDOWN: { return engine::string("The network subsystem has failed."); }
        case WSAEFAULT: { return engine::string("The buf parameter is not completely contained in a valid part of the user address space."); }
        case WSAENOTCONN: { return engine::string("The socket is not connected."); }
        case WSAEINTR: { return engine::string("The(blocking) call was canceled through WSACancelBlockingCall."); }
        case WSAEINPROGRESS: { return engine::string("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."); }
        case WSAENETRESET: { return engine::string("For a connection - oriented socket, this error indicates that the connection has been broken due to keep - alive activity that detected a failure while the operation was in progress.For a datagram socket, this error indicates that the time to live has expired."); }
        case WSAENOTSOCK: { return engine::string("The descriptor is not a socket."); }
        case WSAEOPNOTSUPP: { return engine::string("MSG_OOB was specified, but the socket is not stream - style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations."); }
        case WSAESHUTDOWN: { return engine::string("The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH."); }
        case WSAEWOULDBLOCK: { return engine::string("The socket is marked as nonblocking and the receive operation would block."); }
        case WSAEMSGSIZE: { return engine::string("The message was too large to fit into the specified buffer and was truncated."); }
        case WSAEINVAL: { return engine::string("The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative."); }
        case WSAECONNABORTED: { return engine::string("The virtual circuit was terminated due to a time - out or other failure.The application should close the socket as it is no longer usable."); }
        case WSAETIMEDOUT: { return engine::string("The connection has been dropped because of a network failure or because the peer system failed to respond."); }
        case WSAECONNRESET: { return engine::string("The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket, this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message."); }
        default: return engine::string("socket send unknown error");
        }
    }

    size_t Socket::read(char* buffer, size_t lengthBytes)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t bytesRead = 0;
        auto recv_result = recv(m_socket, buffer, static_cast<int>(lengthBytes), 0);
        if (recv_result == SOCKET_ERROR)
        {
            ASSERT(false, "Socket read failed with error: %s", getReadErrorString(recv_result).c_str());
        }
        else if (recv_result == 0)
        {
            LOG_INFO("Socket gracefully closed");
        }
        else
        {
            bytesRead = recv_result;
        }
        return bytesRead;
    }

    size_t Socket::write(const char* buffer, size_t lengthBytes)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        engine::vector<char> data(lengthBytes);
        memcpy(&data[0], buffer, lengthBytes);
        m_data.emplace_back(DataBlock{ std::move(data), 0 });
        return lengthBytes;
    }

    engine::string getSendErrorString(int errorCode)
    {
        switch (errorCode)
        {
        case WSANOTINITIALISED: { return engine::string("A successful WSAStartup call must occur before using this function."); break; }
        case WSAENETDOWN: { return engine::string("The network subsystem has failed."); break; }
        case WSAEACCES: { return engine::string("The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST socket option to enable use of the broadcast address."); break; }
        case WSAEINTR: { return engine::string("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall."); break; }
        case WSAEINPROGRESS: { return engine::string("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."); break; }
        case WSAEFAULT: { return engine::string("The buf parameter is not completely contained in a valid part of the user address space."); break; }
        case WSAENETRESET: { return engine::string("The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress."); break; }
        case WSAENOBUFS: { return engine::string("No buffer space is available."); break; }
        case WSAENOTCONN: { return engine::string("The socket is not connected."); break; }
        case WSAENOTSOCK: { return engine::string("The descriptor is not a socket."); break; }
        case WSAEOPNOTSUPP: { return engine::string("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations."); break; }
        case WSAESHUTDOWN: { return engine::string("The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH."); break; }
        case WSAEWOULDBLOCK: { return engine::string("The socket is marked as nonblocking and the requested operation would block."); break; }
        case WSAEMSGSIZE: { return engine::string("The socket is message oriented, and the message is larger than the maximum supported by the underlying transport."); break; }

        case WSAEHOSTUNREACH: { return engine::string("The remote host cannot be reached from this host at this time."); break; }
        case WSAEINVAL: { return engine::string("The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled."); break; }
        case WSAECONNABORTED: { return engine::string("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable."); break; }
        case WSAECONNRESET: { return engine::string("The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a 'Port Unreachable' ICMP packet. The application should close the socket as it is no longer usable."); break; }
        case WSAETIMEDOUT: { return engine::string("The connection has been dropped, because of a network failure or because the system on the other end went down without notice."); break; }
        }
        return "socket send unknown error";
    }

    engine::string getConnectErrorString(int errorCode)
    {
        switch (errorCode)
        {
        case WSANOTINITIALISED: { return "A successful WSAStartup call must occur before using this function."; }
        case WSAENETDOWN: { return "The network subsystem has failed."; }
        case WSAEADDRINUSE: { return "The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs when executing bind, but could be delayed until the connect function if the bind was to a wildcard address (INADDR_ANY or in6addr_any) for the local IP address. A specific address needs to be implicitly bound by the connect function."; }
        case WSAEINTR: { return "The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall."; }
        case WSAEINPROGRESS: { return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."; }
        case WSAEALREADY: { return "A nonblocking connect call is in progress on the specified socket."; }
        case WSAEADDRNOTAVAIL: { return "The remote address is not a valid address(such as INADDR_ANY or in6addr_any) ."; }
        case WSAEAFNOSUPPORT: { return "Addresses in the specified family cannot be used with this socket."; }
        case WSAECONNREFUSED: { return "The attempt to connect was forcefully rejected."; }
        case WSAEFAULT: { return "The sockaddr structure pointed to by the name contains incorrect address format for the associated address family or the namelen parameter is too small.This error is also returned if the sockaddr structure pointed to by the name parameter with a length specified in the namelen parameter is not in a valid part of the user address space."; }
        case WSAEINVAL: { return "The parameter s is a listening socket."; }
        case WSAEISCONN: { return "The socket is already connected(connection - oriented sockets only)."; }
        case WSAENETUNREACH: { return "The network cannot be reached from this host at this time."; }
        case WSAEHOSTUNREACH: { return "A socket operation was attempted to an unreachable host."; }
        case WSAENOBUFS: { return "Note  No buffer space is available.The socket cannot be connected."; }
        case WSAENOTSOCK: { return "The descriptor specified in the s parameter is not a socket."; }
        case WSAETIMEDOUT: { return "An attempt to connect timed out without establishing a connection."; }
        case WSAEWOULDBLOCK: { return "The socket is marked as nonblocking and the connection cannot be completed immediately."; }
        case WSAEACCES: { return "An attempt to connect a datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled."; }
        }
        return "unknown socket connect error";
    }

    size_t Socket::private_write()
    {
        size_t result = 0;
        std::lock_guard<std::mutex> lock(m_mutex);
        //for (auto&& data : m_data)
        for(auto iter = m_data.begin(); iter != m_data.end(); ++iter)
        {
            uint32_t bytesToSend = static_cast<uint32_t>((*iter).data.size());
            uint32_t bytesSent = 0;
            while (bytesSent < bytesToSend)
            {
                auto sendResult = send(m_socket, &(*iter).data[0], static_cast<int>((*iter).data.size()), 0);
                if (sendResult == SOCKET_ERROR)
                {
                    auto error = WSAGetLastError();
                    LOG_ERROR("Socket send error: %s", getSendErrorString(error).c_str());

                    if (error == WSAEWOULDBLOCK)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    else
                    {
                        LOG_INFO("Socket encountered error: %s. Erasing the already sent items", getSendErrorString(error).c_str());
                        m_data.erase(m_data.begin(), iter);
                        return result;
                    }
                }
                else if (sendResult > 0)
                {
                    bytesSent += sendResult;
                }
                if (sendResult == 0)
                    LOG_INFO("Sent empty packet");
            }
            result += bytesSent;
        }
        m_data.clear();

        return result;
    }

    engine::string Socket::localHostname()
    {
        // initialize networking
        SocketSystemInitialization::instance().initialize();

        char ac[80];
        if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
        {
            //cerr << "Error " << WSAGetLastError() <<
            //    " when getting local host name." << endl;
            LOG_ERROR("Could not get local hostname");
            return "";
        }
        //LOG_INFO("Host name: %s", ac);

        return ac;
    }

    engine::string Socket::localIp()
    {
        // initialize networking
        SocketSystemInitialization::instance().initialize();

        auto localHost = localHostname();

        struct hostent *phe = gethostbyname(localHost.c_str());
        if (phe == 0) {
            LOG_ERROR("Could not get host by name");
            return "";
        }

        engine::string result = "";
        for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
            struct in_addr addr;
            memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
            auto ipstr = engine::string(inet_ntoa(addr));
            //LOG_INFO("Local IP address: %s", ipstr.c_str());

            if (ipstr.find("127.0.0.1") == engine::string::npos)
            {
                result = ipstr;
            }
        }
        return result;
    }

    engine::shared_ptr<Socket> Socket::createSocket(int port, const engine::string& ip, SocketType type)
    {
        int socketType = SOCK_STREAM;
        switch (type)
        {
        case SocketType::TCP: socketType = SOCK_STREAM; break;
        case SocketType::UDP: socketType = SOCK_DGRAM; break;
        }

        auto res = engine::shared_ptr<Socket>(
            new Socket(socket(PF_INET, socketType, 0)),
            [](Socket* ptr)
        {
            if (ptr->m_socket != INVALID_SOCKET)
            {
                shutdown(ptr->m_socket, SD_BOTH);
                closesocket(ptr->m_socket);
            }
            delete ptr;
        });

        if (socketType == SOCK_DGRAM)
        {
            res->enableBroadcasting();
        }

        memset(res->m_sockAddr.get(), 0, sizeof(sockaddr_in));
        res->m_sockAddr->sin_family = AF_INET;
        res->m_sockAddr->sin_port = htons(static_cast<u_short>(port));
        if(ip == "")
            res->m_sockAddr->sin_addr.s_addr = htonl(INADDR_ANY);
        else
        {
            hostent* he;
            if ((he = gethostbyname(ip.c_str())) == 0)
            {
                LOG_ERROR("Could not get host name");
            }
            res->m_sockAddr->sin_addr = *((in_addr*)he->h_addr);
        }

        return res;
    }

    int Socket::connect()
    {
        auto res = ::connect(m_socket, (struct sockaddr*)m_sockAddr.get(), sizeof(sockaddr));
        if (res == SOCKET_ERROR)
        {
            auto error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK && m_nonBlocking)
            {
                // this is normal behaviour
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                return 0;
            }
            else if (error == WSAEISCONN)
            {
                // we're already connected. everything's okay
                return 0;
            }
            else if (error == WSAEALREADY)
            {
                // we're already connecting. everything's okay.
                return 0;
            }
            else
                LOG_ERROR("Could not connect client socket: %s", connectErrorString(error).c_str());
        }
        return res;
    }

    int Socket::bind()
    {
        auto res = ::bind(m_socket, reinterpret_cast<sockaddr*>(m_sockAddr.get()), sizeof(sockaddr_in));
        if (res == SOCKET_ERROR)
            LOG_ERROR("Could not bind server socket");
        return res;
    }

    /*void Socket::private_read()
    {
        engine::vector<char> m_temporaryData(dataAvailableInSocket());
        auto bytes = recv(m_socket, &m_temporaryData[0], m_temporaryData.size(), 0);
        m_data.emplace_back(DataBlock{ std::move(m_temporaryData), 0 });
    }

    size_t Socket::private_write(char* buffer, size_t lengthBytes)
    {
        return send(m_socket, buffer, lengthBytes, 0);
    }

    size_t Socket::read(char* buffer, size_t lengthBytes)
    {
        char* ptr = buffer;
        for (auto&& data : m_data)
        {
            auto blockAvailableData = data.data.size() - data.currentPointer;
            auto bytesToCopy = min(lengthBytes, blockAvailableData);
            memcpy(ptr, &data.data[data.currentPointer], bytesToCopy);
            data.currentPointer += bytesToCopy;
            ptr += bytesToCopy;
            if (ptr - buffer >= lengthBytes)
                break;
        }

        auto data_iterator = m_data.begin();
        do
        {
            if ((*data_iterator).currentPointer == (*data_iterator).data.size())
            {
                auto temp = data_iterator;
                ++data_iterator;
                m_data.erase(temp);
            }
            else
                ++data_iterator;
        } while (data_iterator != m_data.end());

        return ptr - buffer;
    }

    size_t Socket::write(char* buffer, size_t lengthBytes)
    {
        return send(m_socket, buffer, lengthBytes, 0);
    }*/

    engine::string Socket::ip() const
    {
        struct sockaddr_in m_addr;
#ifdef _WIN32
        int len = sizeof(m_addr);
#else
        socklen_t len = sizeof(m_addr);
#endif
        getpeername(m_socket, (struct sockaddr*)&m_addr, &len);
        return inet_ntoa(m_addr.sin_addr);
    }

    uint32_t Socket::availableBytes() const
    {
        return false;
    }

    void Socket::enableSocketReuse()
    {
        char flag = 1;
        auto res = setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(char));
        if (res == SOCKET_ERROR)
            LOG_ERROR("Could not enable port reuse");
    }

    void Socket::enableNoDelay()
    {
        char flag = 1;
        auto res = setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(char));
#if 1
        if (res == SOCKET_ERROR)
            LOG_ERROR("Could not enable TCP_NODELAY");
#endif
    }

    void Socket::enableNonBlockingSocket()
    {
        u_long arg = 1;
        auto res = ioctlsocket(m_socket, FIONBIO, &arg);
        if (res == SOCKET_ERROR)
            LOG_ERROR("Could not enable non-blocking socket");
        else
            m_nonBlocking = true;
    }

    void Socket::enableBroadcasting()
    {
        char broadcast = 1;
        auto optres = setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(char));
        if (optres == SOCKET_ERROR)
            LOG_ERROR("Could not enable broadcasting on socket");
    }

    uint32_t Socket::dataAvailableInSocket()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        u_long arg = 0;
        auto res = ioctlsocket(m_socket, FIONREAD, &arg);
        if (res == SOCKET_ERROR)
            LOG_ERROR("Could not get available data count");
        return static_cast<uint32_t>(arg);
    }

}
