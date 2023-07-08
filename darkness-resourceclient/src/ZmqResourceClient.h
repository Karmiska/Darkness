#pragma once

#include "zmq.hpp"
#include "ResClientCommon.h"
#include <queue>
#include "containers/vector.h"
#include "containers/memory.h"
#include <thread>
#include <mutex>

#include "protocols/network/ResourceProtocols.pb.h"

class BeaconHello;

namespace engine
{
    class Beacon;
    class Process;
}

namespace resource_client
{
    class ResourceClient
    {
    public:
        ResourceClient(const engine::string& frontIdentity = "");
        bool runService();

        bool hasTasks();
        Task getTask();
        bool hasWorkers();
        void startWorker();
        Worker getWorker();
        void disposeWorker(Worker&& worker);
        void processTask(SplitTask& task);
        void finishTask(
            TaskResult&& task,
            const engine::string& hostTaskId);

        void sendTaskStart(
            const engine::string& hostId,
            const engine::string& hostTaskId);

        void sendTaskFinished(
            const engine::string& hostId,
            const engine::string& hostTaskId);

        void updateTaskProgress(
            const engine::string& hostId,
            const engine::string& hostTaskId,
            float progress);

        void updateTaskProgressMessage(
            const engine::string& hostId,
            const engine::string& hostTaskId,
            float progress,
            const engine::string& message);

        bool shouldShutdown() const;
    private:
        bool m_headless;
        engine::string m_frontIdentity;
        engine::string m_backIdentity;
        engine::shared_ptr<engine::Beacon> m_beacon;
        void onDiscovery(const BeaconHello& hello);
        void onLoss(const BeaconHello& hello);
        zmq::context_t m_context;
        zmq::socket_t m_front;
        zmq::socket_t m_back;
        engine::vector<zmq::pollitem_t> m_polls;
        engine::string m_localIp;
        
        engine::vector<Worker> m_workers;
        engine::vector<Task> m_tasks;

        engine::vector<SplitTask*> m_splitTasks;

        bool m_shouldShutdown;
    };
}
