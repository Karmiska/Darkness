#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include <mutex>

namespace engine
{
    class Process;
    class ResourceDiscoveryServer;
}

namespace platform
{
    class Socket;
    class SocketServer;
    struct NetworkMessage;
}

namespace resource_client_old
{
    static const char* ResourceTaskPath1 = "C:\\work\\darkness\\darkness-resourcetask\\bin\\win64\\debug\\darknessresourcetask.exe";
    static const char* ResourceTaskPath2 = "darknessresourcetask.exe";
    static const char* ResourceTaskArgumentsStart = "--ip=";
    static const char* ResourceTaskArgumentsStop = " --port=2134 --magic=";
    static const char* ResourceTaskWorkingDir = "C:\\work\\darkness\\darkness-resourcetask\\bin\\win64\\debug";

    class ResourceClient
    {
    public:
        ResourceClient();
        void broadcastClientStartup();
        int runService();
    private:
        engine::shared_ptr<platform::SocketServer> m_server;
        engine::shared_ptr<platform::SocketServer> m_serverToHost;
        engine::shared_ptr<engine::ResourceDiscoveryServer> m_discoveryServer;
        void onMessage(engine::shared_ptr<platform::Socket> socket, const platform::NetworkMessage& message);
        void onMessageFromHost(engine::shared_ptr<platform::Socket> socket, const platform::NetworkMessage& message);
        std::mutex m_mutex;
        bool m_alive;

        engine::shared_ptr<platform::Socket> m_masterSocket;

        struct ResourceTaskContainer
        {
            engine::shared_ptr<engine::Process> taskProcess;
            engine::shared_ptr<platform::Socket> socket;
            uint32_t magicNumber;
            engine::vector<char> encodingTaskData;
        };
        uint32_t m_magicNumber;
        engine::string m_localIp;
        engine::vector<ResourceTaskContainer> m_tasks;
    };
}
