#pragma once

#include <functional>
#include "containers/memory.h"
#include "containers/vector.h"

namespace platform
{
    class Socket;

    struct NetworkMessage
    {
        NetworkMessage()
            : buffer{ nullptr }
            , bytes{ 0 }
        {}

        template<typename T>
        NetworkMessage(const engine::vector<T>& data)
            : buffer{ &data[0] }
            , bytes{ data.size() }
        {}

        const char* buffer;
        size_t bytes;
    };

    using OnNetworkMessage = std::function<void(engine::shared_ptr<platform::Socket> socket, const NetworkMessage&)>;

    class PrefixLengthMessageParser
    {
    public:
        PrefixLengthMessageParser(OnNetworkMessage onMessage);
        void sendMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message);

        void setSocket(engine::shared_ptr<platform::Socket> socket);
        void onData(engine::shared_ptr<platform::Socket> socket, const char* buffer, size_t bytes);
    private:
        enum class ParserState
        {
            GetPrefix,
            GetMessage
        };
        engine::shared_ptr<platform::Socket> m_socket;
        OnNetworkMessage m_onMessage;
        ParserState m_parserState;
        uint32_t m_messagePrefix;
        uint32_t m_messagePrefixBytes;
        engine::vector<char> m_messageData;
        uint32_t m_messageDataBytes;
    };
}
