#include "ResourceProcessor.h"
#include "tools/Debug.h"
#include "tools/ProcessorInfo.h"
#include <chrono>

using namespace engine;

namespace resource_client
{
    ResourceProcessor::ResourceProcessor(const engine::string& frontIdentity)
        : m_frontIdentity{ frontIdentity }
        , m_splitter{}
        , m_alive{ true }
        , m_workerMutex{}
        , m_worker{ [this]() { this->runClient(); } }
        , m_client{ nullptr }
        , m_currentTaskCount{ 0 }
        , m_coreCount{ static_cast<int32_t>(engine::getProcessorInfo().processorCoreCount) }
        , m_taskAlive{ true }
    {
    }

    ResourceProcessor::~ResourceProcessor()
    {
        {
			std::lock_guard<std::mutex> lock(m_workerMutex);
            m_alive = false;
        }
        m_worker.join();
    }

    void ResourceProcessor::run(bool async)
    {
        if (!async)
            processTasks();
        else
        {
            m_taskThread = std::move(engine::unique_ptr<std::thread, std::function<void(std::thread*)>>(new std::thread(
                // task
                [this]()
            {
                this->processTasks();
            }),
                // deleter
                [&](std::thread* ptr) {
                    {
						std::lock_guard<std::mutex> lock(m_taskMutex);
                        m_taskAlive = false;
                    }
                    ptr->join();
                    delete ptr;
            }));
        }
    }

    void ResourceProcessor::processTasks()
    {
        // This thread is used for processing task splits
        // It's either the m_taskThread or main thread. depending on headless flag.
        engine::shared_ptr<TaskContainer> split = nullptr;
        bool taskAlive = true;
        while (taskAlive)
        {
            {
				std::lock_guard<std::mutex> lock(m_taskMutex);
                taskAlive = m_taskAlive;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            {
				std::lock_guard<std::mutex> lock(m_workerMutex);
                if (m_tasksForSplitting.size() > 0)
                {
                    split = std::move(m_tasksForSplitting.front());
                    m_tasksForSplitting.pop();
                }
            }
            if (split)
            {
                split->splits = m_splitter.taskSplitter(split->task);

                for (auto&& s : split->splits)
                {
                    split->sourceSizeBytes += s.bytes;
                }

				std::lock_guard<std::mutex> lock(m_workerMutex);
                m_tasksForProcessing.emplace_back(std::move(split));
                split = nullptr;
            }
            
            {
				std::lock_guard<std::mutex> lock(m_workerMutex);
                if (m_tasksForJoining.size() > 0)
                {
                    split = std::move(m_tasksForJoining.front());
                    m_tasksForJoining.pop();
                }
            }
            if (split)
            {
                split->result = m_splitter.taskJoiner(split->splitResults);
                split->result.id = split->task.hostTaskId;
                split->result.hostid = split->task.hostId;
				std::lock_guard<std::mutex> lock(m_workerMutex);
                m_tasksDone.emplace_back(std::move(split));
                split = nullptr;
            }
        }
    }

    int ResourceProcessor::maxActiveTasks() const
    {
        return std::max(m_coreCount - 1, 0);
    }

    void ResourceProcessor::runClient()
    {
        // WORKER Thread
        // This thread is used for communication
        m_client = engine::make_unique<ResourceClient>(m_frontIdentity);
        bool alive = true;
        while (alive)
        {
            {
				std::lock_guard<std::mutex> lock(m_workerMutex);
                alive = m_alive;
            }

            if (m_client->shouldShutdown())
            {
                alive = false;
                {
					std::lock_guard<std::mutex> lock(m_taskMutex);
                    m_taskAlive = false;
                }
            }

            if(!m_client->runService())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (m_client->hasTasks())
            {
				std::lock_guard<std::mutex> lock(m_workerMutex);
                m_tasksForSplitting.push(engine::make_shared<TaskContainer>(m_client->getTask()));
                m_client->sendTaskStart(
                    m_tasksForSplitting.back()->task.hostId,
                    m_tasksForSplitting.back()->task.hostTaskId);
            }

            {
				std::lock_guard<std::mutex> lock(m_workerMutex);
                for(auto task = m_tasksForProcessing.begin(); task != m_tasksForProcessing.end(); ++task)
                {
                    bool taskDone = true;

                    float bytesProcessed = 0u;
                    engine::string progressMsg = "";
                    for (auto&& split : (*task)->splits)
                    {
                        bytesProcessed += split.progress * static_cast<float>(split.bytes);
                        if (split.progressMessage != "")
                        {
                            progressMsg = split.progressMessage;
                        }
                        if (split.requestedWorker && m_client->hasWorkers() && !split.processing)
                        {
                            split.worker = m_client->getWorker();
                            split.processing = true;
                            m_client->processTask(split);
                        }

                        if (!split.requestedWorker && m_currentTaskCount < maxActiveTasks())
                        {
                            m_currentTaskCount++;
                            split.requestedWorker = true;
                            m_client->startWorker();
                        }

                        if (!split.done)
                            taskDone = false;

                        if (split.done && !split.returnedWorker)
                        {
                            --m_currentTaskCount;
                            ASSERT(m_currentTaskCount >= 0, "Current task count went to negative");
                            m_client->disposeWorker(std::move(split.worker));
                            split.returnedWorker = true;
                        }

                    }

                    if (taskDone)
                    {
                        for (auto&& split : (*task)->splits)
                        {
                            (*task)->splitResults.emplace_back(std::move(split.result));
                        }
                        m_tasksForJoining.push(std::move(*task));
                        m_tasksForProcessing.erase(task);
                        break;
                    }
                    else
                    {
                        float allBytes = static_cast<float>((*task)->sourceSizeBytes);
                        float allProgress = bytesProcessed / allBytes;
                        if ((*task)->task.lastProgressUpdateValue != allProgress ||
                            (*task)->task.lastProgressMessageUpdateValue != progressMsg)
                        {
                            (*task)->task.lastProgressUpdateValue = allProgress;
                            (*task)->task.lastProgressMessageUpdateValue = progressMsg;
                            if ((*task)->task.lastProgressMessageUpdateValue != "")
                            {
                                m_client->updateTaskProgressMessage(
                                    (*task)->task.hostId,
                                    (*task)->task.hostTaskId,
                                    allProgress,
                                    progressMsg);
                            }
                            else
                                m_client->updateTaskProgress(
                                    (*task)->task.hostId,
                                    (*task)->task.hostTaskId,
                                    allProgress);
                        }
                    }
                }

                for (auto task = m_tasksDone.begin(); task != m_tasksDone.end(); ++task)
                {
                    m_client->finishTask(
                        std::move((*task)->result),
                        (*task)->task.hostTaskId);

                    m_client->sendTaskFinished(
                        (*task)->task.hostId,
                        (*task)->task.hostTaskId);
                }
                m_tasksDone.clear();
            }
        }
        m_client = nullptr;
    }
}
