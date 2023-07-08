#pragma once

#include "SocketCallbacks.h"
#include "PrefixLengthMessageParser.h"
#include "containers/string.h"

#include <thread>
#include <mutex>
#include "containers/vector.h"

namespace platform
{
    class SocketServer
    {
    public:
        SocketServer(
            int port, 
            bool blocking,
            bool broadcast,
            OnConnect onConnect, 
            OnDisconnect onDisconnect,
            OnNetworkMessage onData);

        void sendMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message);
        engine::string ip() const;
    private:
        int m_port;
        bool m_broadcasting;
        OnConnect           m_onConnect;
        OnDisconnect        m_onDisconnect;
        OnNetworkMessage    m_onData;

        PrefixLengthMessageParser m_serverSocketParser;
        engine::shared_ptr<Socket> m_serverSocket;
        engine::unique_ptr<std::thread, std::function<void(std::thread*)>> m_work;
        std::mutex m_mutex;
        bool m_alive;
        void run(std::mutex& mutex);

        struct ClientContainer
        {
            engine::shared_ptr<PrefixLengthMessageParser> parser;
            engine::shared_ptr<Socket> socket;
        };

        engine::vector<ClientContainer> m_clients;

        engine::unique_ptr<char, std::function<void(char*)>> m_closingDownMessage;
    };
}
