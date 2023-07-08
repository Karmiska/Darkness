#pragma once

#include "containers/memory.h"
#include "containers/string.h"
#include "containers/vector.h"
#include <functional>

namespace platform
{
    class Socket;
    class SocketClient;
    class SocketServer;
    struct NetworkMessage;
}

namespace engine
{
    enum class DiscoveryMessage;

    using OnNetworkResourceDiscovery = std::function<void(const engine::string&)>;

    class ResourceDiscoveryServer
    {
    public:
        ResourceDiscoveryServer(OnNetworkResourceDiscovery onDiscovery);
    private:
        OnNetworkResourceDiscovery m_onDiscovery;
        engine::shared_ptr<platform::SocketClient> m_client;
        engine::shared_ptr<platform::SocketServer> m_server;
        void onClientMessage(engine::shared_ptr<platform::Socket> socket, const platform::NetworkMessage& message);
        void onServerMessage(engine::shared_ptr<platform::Socket> socket, const platform::NetworkMessage& message);
        engine::vector<engine::string> m_discoveredClients;
    };
}
