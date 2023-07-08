#include "engine/resources/ResourceDiscoveryServer.h"
#include "engine/resources/ResourceMessages.h"
#include "platform/network/PrefixLengthMessageParser.h"
#include "platform/network/Socket.h"
#include "platform/network/SocketClient.h"
#include "platform/network/SocketServer.h"
#include "tools/Debug.h"

using namespace platform;
using namespace engine;

namespace engine
{
    ResourceDiscoveryServer::ResourceDiscoveryServer(OnNetworkResourceDiscovery onDiscovery)
        : m_onDiscovery{ onDiscovery }
        , m_client{ engine::make_shared<SocketClient>("255.255.255.255", 21343, false, true,
            [](engine::shared_ptr<Socket> socket) { /*LOG_INFO("Client connected");*/ },
            [](engine::shared_ptr<Socket> socket) { /*LOG_INFO("Client disconnected");*/ },
            [this](engine::shared_ptr<Socket> socket, const platform::NetworkMessage& message)
            {
                this->onClientMessage(socket, message);
            })}
        , m_server{ engine::make_shared<SocketServer>(21343, false, true,
            [](engine::shared_ptr<Socket> socket) { /*LOG("Connection from: %s", socket->ip().c_str());*/ },
            [](engine::shared_ptr<Socket> socket) { /*LOG("Client %s disconnected", socket->ip().c_str());*/ },
            [this](engine::shared_ptr<Socket> socket, const NetworkMessage& message)
            {
                this->onServerMessage(socket, message);
            })}
    {
        auto localIp = Socket::localIp();
        uint32_t len = static_cast<uint32_t>(localIp.length());
        DiscoveryMessage disMsg = DiscoveryMessage::Ehlo;

        vector<char> outmessage(sizeof(DiscoveryMessage) + sizeof(uint32_t) + len);
        memcpy(&outmessage[0], &disMsg, sizeof(DiscoveryMessage));
        memcpy(&outmessage[sizeof(DiscoveryMessage)], &len, sizeof(uint32_t));
        memcpy(&outmessage[sizeof(DiscoveryMessage) + sizeof(uint32_t)], localIp.data(), len);
        m_client->sendMessage(m_client->socket(), outmessage);

        LOG_INFO("Discovery server started");
    }

    void ResourceDiscoveryServer::onClientMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message)
    {
        ASSERT(message.bytes >= sizeof(DiscoveryMessage), "Malformed Resource client message");
    }

    void ResourceDiscoveryServer::onServerMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message)
    {
        ASSERT(message.bytes >= sizeof(DiscoveryMessage), "Malformed Resource client message");
        DiscoveryMessage msg;
        memcpy(&msg, message.buffer, sizeof(DiscoveryMessage));
        switch (msg)
        {
            case DiscoveryMessage::Helo:
            {
                auto localIp = Socket::localIp();
                uint32_t len = static_cast<uint32_t>(localIp.length());
                DiscoveryMessage disMsg = DiscoveryMessage::Ehlo;

                vector<char> outmessage(sizeof(DiscoveryMessage) + sizeof(uint32_t) + len);
                memcpy(&outmessage[0], &disMsg, sizeof(DiscoveryMessage));
                memcpy(&outmessage[sizeof(DiscoveryMessage)], &len, sizeof(uint32_t));
                memcpy(&outmessage[sizeof(DiscoveryMessage) + sizeof(uint32_t)], localIp.data(), len);
                m_client->sendMessage(m_client->socket(), outmessage);

                auto clientIp = m_client->socket()->ip();
                //if (std::find(m_discoveredClients.begin(), m_discoveredClients.end(), clientIp) == m_discoveredClients.end())
                {
                    m_discoveredClients.emplace_back(clientIp);
                    if (m_onDiscovery)
                        m_onDiscovery(clientIp);
                }

                break;
            }
        }
    }
}
