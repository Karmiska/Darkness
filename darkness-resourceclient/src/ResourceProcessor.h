#pragma once

#include <thread>
#include <mutex>
#include "containers/memory.h"
#include <queue>

#include "TaskSplitter.h"
#include "ZmqResourceClient.h"

namespace resource_client
{
    class ResourceProcessor
    {
    public:
        ResourceProcessor(const engine::string& frontIdentity = "");
        ~ResourceProcessor();
        void run(bool async = false);
    private:
        engine::string m_frontIdentity;
        TaskSplitter m_splitter;
        volatile bool m_alive;
        std::mutex m_workerMutex;
        std::thread m_worker;
        engine::unique_ptr<ResourceClient> m_client;
        int32_t m_currentTaskCount;
        int32_t m_coreCount;

        int maxActiveTasks() const;

        void processTasks();
        void runClient();
        
        std::mutex m_taskMutex;
        volatile bool m_taskAlive;
        engine::unique_ptr<std::thread, std::function<void(std::thread*)>> m_taskThread;

        struct TaskContainer
        {
            Task task;
            engine::vector<SplitTask> splits;
            engine::vector<SplitTaskResult> splitResults;
            TaskResult result;
            float progress;
            uint64_t sourceSizeBytes;
            TaskContainer(Task _task)
                : task{ _task }
                , splits{}
                , splitResults{}
                , result{}
                , progress{ 0.0f }
                , sourceSizeBytes{ 0u }
            {}
        };
        std::queue <engine::shared_ptr<TaskContainer>> m_tasksForSplitting;
        engine::vector<engine::shared_ptr<TaskContainer>> m_tasksForProcessing;
        std::queue <engine::shared_ptr<TaskContainer>> m_tasksForJoining;
        engine::vector<engine::shared_ptr<TaskContainer>> m_tasksDone;
    };
}