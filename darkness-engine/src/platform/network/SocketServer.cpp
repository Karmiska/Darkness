#include "platform/network/SocketServer.h"
#include "platform/network/Socket.h"
#include "platform/network/SocketSystemInitialization.h"
#include "tools/Debug.h"

#ifdef _WIN32
#include <winsock.h>
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
    SocketServer::SocketServer(
        int                 port,
        bool                blocking,
        bool                broadcast,
        OnConnect           onConnect,
        OnDisconnect        onDisconnect,
        OnNetworkMessage    onData)
        : m_port{ port }
        , m_broadcasting{ broadcast }
        , m_alive{ true }
        , m_onConnect{ onConnect }
        , m_onDisconnect{ onDisconnect }
        , m_onData{ onData }
        , m_serverSocketParser{ m_onData }
    {
        // initialize networking
        SocketSystemInitialization::instance().initialize();

        m_serverSocket = Socket::createSocket(port, "", !broadcast ? SocketType::TCP : SocketType::UDP);

        if (m_serverSocket->m_socket == INVALID_SOCKET)
        {
            LOG_ERROR("Could not open socket");
            return;
        }

        m_serverSocket->enableSocketReuse();
        m_serverSocket->enableNoDelay();
        if(!blocking)
            m_serverSocket->enableNonBlockingSocket();

        auto res = m_serverSocket->bind();
        if (res == SOCKET_ERROR)
        {
            LOG_ERROR("Could not bind socket");
            return;
        }

        listen(m_serverSocket->m_socket, 1024);

        LOG_INFO("SocketServer started on host: %s, ip: %s", Socket::localHostname().c_str(), Socket::localIp().c_str());

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
                    delete ptr;
                }));
            if (!SetThreadPriority(m_work->native_handle(), THREAD_PRIORITY_HIGHEST))
            {
                LOG_ERROR("Could not set thread priority");
            }
        }
        else
            run(m_mutex);

        m_closingDownMessage = engine::unique_ptr<char, std::function<void(char*)>>(new char(), [](char* ptr) { LOG_INFO("SocketServer stopped"); delete ptr; });
    }

    /*engine::shared_ptr<Socket>& SocketServer::socket()
    {
        return m_serverSocket;
    }*/

    void SocketServer::sendMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message)
    {
        for (auto&& client : m_clients)
        {
            if (client.socket == socket)
            {
                client.parser->sendMessage(socket, message);
                return;
            }
        }
        m_serverSocketParser.sendMessage(socket, message);
    }

    engine::string SocketServer::ip() const
    {
        return m_serverSocket->ip();
    }

    void SocketServer::run(std::mutex& mutex)
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
            FD_SET(m_serverSocket->m_socket, &reader);
            if(m_serverSocket->m_data.size() > 0)
                FD_SET(m_serverSocket->m_socket, &writer);

            // set client sockets
            for (auto&& sock : m_clients)
            {
                FD_SET(sock.socket->m_socket, &reader);
                if(sock.socket->m_data.size() > 0)
                //if (sock->hasData())
                    FD_SET(sock.socket->m_socket, &writer);
            }

            timeval time;
            time.tv_sec = 0;
            time.tv_usec = 1000; // 500 ms
            auto changedSockets = select(0, &reader, &writer, nullptr, &time);
            if (changedSockets == SOCKET_ERROR)
                LOG_ERROR("Socket select failed");

            // check for new connections
            if (FD_ISSET(m_serverSocket->m_socket, &reader))
            {
                if(!m_broadcasting)
                {
                    // new connection
                    LOG_WARNING("SocketServer got a new connection");
                    m_clients.emplace_back(ClientContainer{
                        engine::make_shared<PrefixLengthMessageParser>(m_onData),
                        engine::shared_ptr<Socket>(
                        new Socket(accept(m_serverSocket->m_socket, 0, 0)),
                        [](Socket* ptr) { if(ptr->m_socket != INVALID_SOCKET)
                        { closesocket(ptr->m_socket); } delete ptr; })});

                    SocketServer::ClientContainer& client = m_clients.back();
                    if (client.socket->m_socket == INVALID_SOCKET)
                    {
                        LOG_ERROR("Client socket failed");
                        m_clients.pop_back();
                    }
                    else
                    {
                        client.parser->setSocket(client.socket);
                        client.socket->enableSocketReuse();
                        client.socket->enableNoDelay();
                        client.socket->enableNonBlockingSocket();
                        if (m_onConnect)
                            m_onConnect(client.socket);
                    }
                }
                else
                {
                    auto dataSize = m_serverSocket->dataAvailableInSocket();
                    if (dataSize > 0)
                    {
                        //LOG_INFO("got data: %i", static_cast<int>(dataSize));
                        engine::vector<char> buffer(dataSize);
                        auto readBytes = m_serverSocket->read(&buffer[0], dataSize);
                        m_serverSocketParser.onData(m_serverSocket, &buffer[0], readBytes);
                        performedRead = true;
                        //client.parser->onData(client.socket, &buffer[0], readBytes);
                    }
                }
            }

            // perform client reads/writes
            engine::vector<ClientContainer> remove;
            for (auto&& client : m_clients)
            {
                if (FD_ISSET(client.socket->m_socket, &reader))
                {
                    size_t bytesRead = 0;
                    auto dataSize = client.socket->dataAvailableInSocket();
                    engine::vector<char> buffer(dataSize);
                    while(bytesRead < dataSize)
                    {
                        //LOG_INFO("got data: %i", static_cast<int>(dataSize));
                        
                        bytesRead += client.socket->read(&buffer[bytesRead], dataSize - bytesRead);
                        
                        performedRead = true;
                    }
                    ASSERT(bytesRead == dataSize, "Something went bad");
                    if(bytesRead > 0)
                        client.parser->onData(client.socket, &buffer[0], bytesRead);
                    
                    if(dataSize == 0)
                    {
                        //LOG_INFO("got 0 packet");
                        // client disconnected
                        remove.emplace_back(client);
                    }
                }
                if (FD_ISSET(client.socket->m_socket, &writer))
                {
                    if(client.socket->private_write() > 0)
                        performedWrite = true;
                }
            }
            /*while (remove.size() > 0)
            {
                auto sock = remove.back();
                if (m_onDisconnect)
                    m_onDisconnect(sock.socket);
                remove.pop_back();
                for (auto iter = m_clients.begin(); iter != m_clients.end(); ++iter)
                {
                    if ((*iter).socket == sock.socket)
                    {
                        m_clients.erase(iter);
                        break;
                    }
                }
                
            }*/

            if (!performedRead && !performedWrite)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    }
}
