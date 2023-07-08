#pragma once

#include "containers/vector.h"
#include "containers/string.h"
#include "zmq.hpp"

namespace engine
{
    class MqMessage
    {
    public:
        MqMessage();
        MqMessage(zmq::socket_t& socket);

        MqMessage(MqMessage&& msg)
            : m_parts{ std::move(msg.m_parts) }
        {}
        MqMessage& operator=(MqMessage&& msg)
        {
            m_parts = std::move(msg.m_parts);
            return *this;
        }

        MqMessage(const MqMessage&) = delete;
        MqMessage& operator=(const MqMessage&) = delete;

        void prepend(const char* msg);
        void prepend(engine::vector<char>&& msg);
        void prepend(const engine::string& msg);

        void emplace_back(const char* msg);
        void emplace_back(engine::vector<char>&& msg);
        void emplace_back(const engine::string& msg);
        void send(zmq::socket_t& socket);

        const engine::vector<engine::vector<char>>& parts() const;
        engine::vector<engine::vector<char>>& parts();
    private:
        engine::vector<engine::vector<char>> m_parts;
    };
}
