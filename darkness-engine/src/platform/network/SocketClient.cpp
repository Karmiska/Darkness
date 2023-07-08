#include "platform/network/SocketClient.h"
#include "platform/network/Socket.h"
#include "platform/network/SocketSystemInitialization.h"
#include "tools/Debug.h"

#ifdef _WIN32
//#include <winsock.h>
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
    SocketClient::SocketClient(
        const engine::string& ip,
        int port,
        bool blocking,
        bool broadcast,
        OnConnect onConnect,
        OnDisconnect onDisconnect,
        OnNetworkMessage onData)
        : m_port{ port }
        , m_alive{ true }
        , m_onConnect{ onConnect }
        , m_onDisconnect{ onDisconnect }
        , m_onData{ onData }
    {
        // initialize networking
        SocketSystemInitialization::instance().initialize();

        m_clientSocket = Socket::createSocket(port, ip, !broadcast ? SocketType::TCP : SocketType::UDP);
        m_parser = engine::make_shared<PrefixLengthMessageParser>(m_onData);
        m_parser->setSocket(m_clientSocket);

        if (m_clientSocket->m_socket == INVALID_SOCKET)
        {
            LOG_ERROR("Could not open socket");
            return;
        }

        m_clientSocket->enableSocketReuse();
        m_clientSocket->enableNoDelay();
        if (!blocking)
            m_clientSocket->enableNonBlockingSocket();

        auto res = m_clientSocket->connect();
        if (res == SOCKET_ERROR)
        {
            LOG_ERROR("Could not connect to socket: %s", getConnectErrorString(WSAGetLastError()).c_str());

            return;
        }

        //LOG_INFO("SocketClient started");

        if (m_onConnect)
            m_onConnect(m_clientSocket);

        if (!blocking)
        {
            m_work = std::move(engine::unique_ptr<std::thread, std::function<void(std::thread*)>>(new std::thread(
                // task
                [&]()
                {
                    run(m_mutex);
                }),

                // deleter
                [&](std::thread* ptr) {
                    {
                        std::lock_guard<std::mutex> m(m_mutex);
                        m_alive = false;
                    }
                    ptr->join();
                    if (m_onDisconnect)
                        m_onDisconnect(m_clientSocket);
                    delete ptr;
            }));
            if (!SetThreadPriority(m_work->native_handle(), THREAD_PRIORITY_HIGHEST))
            {
                LOG_ERROR("Could not set thread priority");
            }
        }
        else
            run(m_mutex);

        m_closingDownMessage = engine::unique_ptr<char, std::function<void(char*)>>(new char(), [](char* ptr)
        {
            //LOG_INFO("SocketClient stopped"); 
            delete ptr;
        });
    }

    /*void SocketClient::write(const char* buffer, size_t bytes)
    {
        m_clientSocket->write(buffer, bytes);
    }*/

    void SocketClient::sendMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message)
    {
        m_parser->sendMessage(socket, message);
    }

    engine::shared_ptr<Socket>& SocketClient::socket()
    {
        return m_clientSocket;
    }

    void SocketClient::run(std::mutex& mutex)
    {
        bool alive = true;
        while (alive)
        {
            bool performedRead = false;
            bool performedWrite = false;

            {
                std::lock_guard<std::mutex> m(mutex);
                alive = m_alive;
            }
            if (!alive)
                break;

            fd_set reader;
            fd_set writer;
            FD_ZERO(&reader);
            FD_ZERO(&writer);

            // set server socket
            FD_SET(m_clientSocket->m_socket, &reader);
            FD_SET(m_clientSocket->m_socket, &writer);

            timeval time;
            time.tv_sec = 0;
            time.tv_usec = 1000; // 10 ms
            auto changedSockets = select(0, &reader, &writer, nullptr, &time);
            if (changedSockets == SOCKET_ERROR)
                LOG_ERROR("Socket select failed");

            // perform client reads/writes
            if (FD_ISSET(m_clientSocket->m_socket, &reader))
            {
                auto dataSize = m_clientSocket->dataAvailableInSocket();
                if (dataSize > 0)
                {
                    engine::vector<char> buffer(dataSize);
                    auto bytesRead = m_clientSocket->read(&buffer[0], dataSize);
                    m_parser->onData(m_clientSocket, &buffer[0], bytesRead);
                    performedRead = true;
                }
                else
                {
                    return;
                }
            }

            if (FD_ISSET(m_clientSocket->m_socket, &writer))
            {
                if(m_clientSocket->private_write() > 0)
                    performedWrite = true;
            }

            if (!performedRead && !performedWrite)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
        //m_clientSocket->send_eof();
    }
}
