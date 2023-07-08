#pragma once

#include "engine/graphics/Format.h"
#include "engine/resources/ResourceCommon.h"
#include "engine/network/NetworkCommunication.h"

#include "containers/vector.h"
#include "containers/unordered_map.h"
#include <functional>
#include "containers/memory.h"
#include <mutex>
#include <deque>

namespace platform
{
    class Socket;
    class SocketClient;
    struct NetworkMessage;
}

namespace engine
{
    class Process;
    
#ifdef _DEBUG
    static const char* ResourceClientPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\debug\\DarknessResourceClient.exe";
    static const char* ResourceClientWorkingPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\debug";
    
    static const char* ResourceClientFallbackPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\release\\DarknessResourceClient.exe";
    static const char* ResourceClientFallbackWorkingPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\release";
#else
    static const char* ResourceClientPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\release\\DarknessResourceClient.exe";
    static const char* ResourceClientWorkingPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\release";
    
    static const char* ResourceClientFallbackPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\debug\\DarknessResourceClient.exe";
    static const char* ResourceClientFallbackWorkingPath = "..\\..\\..\\..\\darkness-resourceclient\\bin\\win64\\debug";
#endif

    enum class TaskMessageType
    {
        TaskStarted,
        TaskDone
    };

    constexpr int FakeStartMaxSize = 5;

    class ResourceHost
    {
    public:
        ResourceHost();
        ~ResourceHost();

        void processResources(const ProcessResourcePackage& package);

    private:
        std::mutex m_taskIdMutex;

        // delayed start
        engine::unique_ptr<std::thread, std::function<void(std::thread*)>> m_delayedLocalResourceClientStart;
        void delayedStart();
        void startLocalResourceClient();
        void stopLocalResourceClient();
        engine::shared_ptr<Process> m_resourceClientProcess;
        volatile bool m_localResourceProcess;
        engine::string m_localResourceProcessId;

        // TASK STUFF
        engine::unique_ptr<std::thread, std::function<void(std::thread*)>> m_taskThread;
        std::mutex m_taskMutex;
        struct ProcessingItem
        {
            engine::vector<ProcessResourceItem> items;
            ResourceProcessingStarted onStarted;
            ResourceProcessingFinished onFinished;
            ResourceProcessingProgress onProgress;
            ResourceProcessingProgressMessage onProgressMessage;
        };
        std::deque<ProcessingItem> m_taskQueue;
        ProcessingItem m_workItem;
        volatile bool m_taskAlive;
        volatile bool m_taskWorkedOn;
        void taskHandler();

        struct ProcessingTask
        {
            engine::ProcessResourceItem item;
            engine::vector<char> data;
            engine::string id;
        };
        std::mutex m_tasksListMutex;
        engine::unordered_map<engine::string, engine::unique_ptr<ProcessingTask>> m_tasks;

        void sendTask(BeaconHello& beacon, ProcessingTask* task);

        engine::vector<char> readFile(const engine::string& path);

        engine::unique_ptr<std::thread, std::function<void(std::thread*)>> m_taskWorkerThread;
        void taskWorkerHandler();

        NetworkCommunication m_netComm;
        std::mutex m_clientMutex;
        struct ClientContainer
        {
            BeaconHello client;
            uint32_t cores;
            engine::vector<engine::string> tasksActive;
        };
        engine::vector<ClientContainer> m_knownClients;
        BeaconHello getAvailableClient(const engine::string& taskId);

        void forceItemVisibility();
        engine::vector<ProcessResourceItem> m_visibleItems;
    };
}
