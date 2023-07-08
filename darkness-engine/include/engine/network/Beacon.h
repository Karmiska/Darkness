#pragma once

#include "containers/memory.h"
#include "containers/string.h"
#include "containers/vector.h"
#include <functional>
#include <mutex>
#include <thread>
#include <chrono>

#include "protocols/network/BeaconProtocols.pb.h"

namespace platform
{
    class Socket;
    class SocketClient;
    class SocketServer;
    struct NetworkMessage;
}

namespace engine
{
    using OnNetworkResourceDiscovery = std::function<void(const BeaconHello&)>;
    using OnNetworkResourceLoss = std::function<void(const BeaconHello&)>;

    enum class BeaconType
    {
        Server,
        Client
    };

    class BeaconHelloContainer : public BeaconHello
    {
    public:
        BeaconHelloContainer(const BeaconHello& hello)
            : BeaconHello{ hello }
            , lastSeen{ std::chrono::high_resolution_clock::now() }
        {}
        std::chrono::time_point<std::chrono::high_resolution_clock> lastSeen;
    };

    class Beacon
    {
    public:
        Beacon(
            BeaconType type, 
            const engine::string& identity,
            OnNetworkResourceDiscovery onDiscovery,
            OnNetworkResourceLoss onLoss,
            int keepAliveNotifyMilliSeconds = 2000,
            int keepAliveMilliSeconds = 10000);
        ~Beacon();
    private:
        BeaconType m_type;
        engine::string m_identity;
        OnNetworkResourceDiscovery m_onDiscovery;
        OnNetworkResourceLoss m_onLoss;
        int m_keepAliveNotifyMilliSeconds;
        int m_keepAliveMilliSeconds;
        engine::string m_localIp;
        engine::shared_ptr<platform::SocketClient> m_client;
        engine::shared_ptr<platform::SocketServer> m_server;
        void onClientMessage(engine::shared_ptr<platform::Socket> socket, const platform::NetworkMessage& message);
        void onServerMessage(engine::shared_ptr<platform::Socket> socket, const platform::NetworkMessage& message);
        engine::vector<engine::string> m_discoveredClients;

        engine::vector<BeaconHelloContainer> m_knownServers;
        engine::vector<BeaconHelloContainer> m_knownClients;
        engine::vector<BeaconHelloContainer> m_onHoldLeaving;

        bool isMe(const BeaconHello& hello);
        bool knownClient(const BeaconHello& hello);

        std::mutex m_mutex;
        engine::unique_ptr<std::thread, std::function<void(std::thread*)>> m_thread;
        volatile bool m_alive;
        void onKeepalive();
    };
}
