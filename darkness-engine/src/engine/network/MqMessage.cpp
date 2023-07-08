#include "engine/network/MqMessage.h"
#include "tools/Debug.h"

using namespace engine;
using namespace zmq;

namespace engine
{
    MqMessage::MqMessage()
    {}

    MqMessage::MqMessage(socket_t& socket)
    {
        bool more = true;
        while (more)
        {
            message_t msg;
            socket.recv(&msg);
            if (msg.size() > 0)
            {
                vector<char> data(msg.size());
                memcpy(&data[0], msg.data(), data.size());
                m_parts.emplace_back(std::move(data));
            }

            more = socket.getsockopt<int64_t>(static_cast<int>(ZMQ_RCVMORE)) != 0;
        }
    }

    void MqMessage::prepend(const char* msg)
    {
        auto len = strlen(msg);
        if (len > 0)
        {
            engine::vector<char> data(len);
            memcpy(&data[0], msg, data.size());
            message_t m(data.data(), data.size());
            m_parts.insert(m_parts.begin(), move(data));
        }
        else
        {
            engine::vector<char> data;
            m_parts.insert(m_parts.begin(), std::move(data));
        }
    }

    void MqMessage::prepend(vector<char>&& msg)
    {
        m_parts.insert(m_parts.begin(), move(msg));
    }

    void MqMessage::prepend(const string& msg)
    {
        engine::vector<char> data(msg.size());
        memcpy(&data[0], msg.data(), data.size());
        message_t m(data.data(), data.size());
        m_parts.insert(m_parts.begin(), move(data));
    }

    void MqMessage::emplace_back(const char* msg)
    {
        auto len = strlen(msg);
        if (len > 0)
        {
            engine::vector<char> data(len);
            memcpy(&data[0], msg, data.size());
            message_t m(data.data(), data.size());
            m_parts.emplace_back(move(data));
        }
        else
        {
            engine::vector<char> data;
            m_parts.emplace_back(std::move(data));
        }
    }

    void MqMessage::emplace_back(vector<char>&& msg)
    {
        m_parts.emplace_back(move(msg));
    }

    void MqMessage::emplace_back(const string& msg)
    {
        engine::vector<char> data(msg.size());
        memcpy(&data[0], msg.data(), data.size());
        message_t m(data.data(), data.size());
        m_parts.emplace_back(move(data));
    }

    void MqMessage::send(socket_t& socket)
    {
        for(int i = 0; i < m_parts.size(); ++i)
        {
            int flag = i == m_parts.size() - 1 ? 0 : ZMQ_SNDMORE;
            try
            {
                if (m_parts[i].size() > 0)
                    socket.send(message_t(&m_parts[i][0], m_parts[i].size()), flag);
                else
                    socket.send(message_t(), flag);
            }
            catch(const std::exception& /*e*/)
            {
                switch (errno)
                {
                case EAGAIN: LOG_ERROR("Non-blocking mode was requested and the message cannot be sent at the moment."); break;
                case ENOTSUP: LOG_ERROR("The zmq_send() operation is not supported by this socket type."); break;
                case EFSM: LOG_ERROR("The zmq_send() operation cannot be performed on this socket at the moment due to the socket not being in the appropriate state. This error may occur with socket types that switch between several states, such as ZMQ_REP. See the messaging patterns section of zmq_socket(3) for more information."); break;
                case ETERM: LOG_ERROR("The ØMQ context associated with the specified socket was terminated."); break;
                case ENOTSOCK: LOG_ERROR("The provided socket was invalid."); break;
                case EINTR: LOG_ERROR("The operation was interrupted by delivery of a signal before the message was sent."); break;
                case EFAULT: LOG_ERROR("Invalid message."); break;
                case EHOSTUNREACH: LOG_ERROR("Host unreachable"); break;
                default: ASSERT(false, "Unhandled network error occurred");
                }
            }
        }
    }

    const engine::vector<engine::vector<char>>& MqMessage::parts() const
    {
        return m_parts;
    }
    
    engine::vector<engine::vector<char>>& MqMessage::parts()
    {
        return m_parts;
    }
}
