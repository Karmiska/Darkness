#pragma once

#include "SocketCallbacks.h"
#include "PrefixLengthMessageParser.h"
#include "containers/string.h"
#include <mutex>
#include "containers/vector.h"

namespace platform
{
    class SocketClient
    {
    public:
        SocketClient(
            const engine::string& ip,
            int port,
            bool blocking,
            bool broadcast,
            OnConnect onConnect,
            OnDisconnect onDisconnect,
            OnNetworkMessage onData);
        void sendMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message);
        //void write(const char* buffer, size_t bytes);

        engine::shared_ptr<Socket>& socket();
    private:
        int m_port;
        OnConnect           m_onConnect;
        OnDisconnect        m_onDisconnect;
        OnNetworkMessage    m_onData;

        engine::shared_ptr<PrefixLengthMessageParser> m_parser;
        engine::shared_ptr<Socket> m_clientSocket;

        engine::unique_ptr<std::thread, std::function<void(std::thread*)>> m_work;
        std::mutex m_mutex;
        bool m_alive;
        void run(std::mutex& mutex);
        engine::vector<engine::shared_ptr<Socket>> m_clients;
        engine::unique_ptr<char, std::function<void(char*)>> m_closingDownMessage;
    };
}
