#include "platform/network/PrefixLengthMessageParser.h"
#include "platform/network/Socket.h"
#include "tools/Debug.h"
#include <algorithm>

namespace platform
{
    PrefixLengthMessageParser::PrefixLengthMessageParser(
        OnNetworkMessage onMessage)
        : m_onMessage{ onMessage }
        , m_parserState{ ParserState::GetPrefix }
        , m_messagePrefix{ 0 }
        , m_messagePrefixBytes{ 0 }
        , m_messageDataBytes{ 0 }
    {
    }

    void PrefixLengthMessageParser::setSocket(engine::shared_ptr<platform::Socket> socket)
    {
        m_socket = socket;
    }

    void PrefixLengthMessageParser::sendMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message)
    {
        ASSERT(message.bytes > 0, "Weird message");
        engine::vector<char> buffer(message.bytes + sizeof(uint32_t));
        memcpy(&buffer[0], &message.bytes, sizeof(uint32_t));
        memcpy(&buffer[sizeof(uint32_t)], message.buffer, message.bytes);
        //LOG_INFO("PrefixParser: %p sending %u bytes", this, buffer.size() - sizeof(uint32_t));
        socket->write(&buffer[0], buffer.size());
    }

    void PrefixLengthMessageParser::onData(engine::shared_ptr<platform::Socket> socket, const char* buffer, size_t bytes)
    {
        uint32_t inputBytePosition = 0;
        while (inputBytePosition < bytes)
        {
            switch (m_parserState)
            {
                case ParserState::GetPrefix:
                {
                    auto readBytes = std::min(static_cast<uint32_t>(bytes - inputBytePosition), static_cast<uint32_t>(sizeof(uint32_t) - m_messagePrefixBytes));
                    memcpy(reinterpret_cast<char*>(&m_messagePrefix) + m_messagePrefixBytes, buffer + inputBytePosition, readBytes);
                    m_messagePrefixBytes += readBytes;
                    inputBytePosition += readBytes;

                    if (m_messagePrefixBytes == sizeof(uint32_t))
                    {
                        if (m_messagePrefix > 0)
                        {
                            //LOG_INFO("PrefixParser: %p received %u bytes", this, m_messagePrefix);
                            m_messageData.resize(m_messagePrefix);
                            m_parserState = ParserState::GetMessage;
                        }
                        else
                        {
                            ASSERT(false, "Got zero message. Shouldn't be possible");
                        }
                        m_messagePrefixBytes = 0;
                    }
                    break;
                }
                case ParserState::GetMessage:
                {
                    auto readBytes = std::min(static_cast<uint32_t>(bytes - inputBytePosition), static_cast<uint32_t>(m_messageData.size() - m_messageDataBytes));
                    memcpy(&m_messageData[0] + m_messageDataBytes, buffer + inputBytePosition, readBytes);
                    m_messageDataBytes += readBytes;
                    inputBytePosition += m_messageDataBytes;

                    if (m_messageData.size() == m_messageDataBytes)
                    {
                        m_messageDataBytes = 0;
                        if (m_onMessage)
                            m_onMessage(socket, NetworkMessage{ m_messageData });
                        m_messageData.clear();
                        m_parserState = ParserState::GetPrefix;
                    }
                    break;
                }
            }
        }
    }
}
