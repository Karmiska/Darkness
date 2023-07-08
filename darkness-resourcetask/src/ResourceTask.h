#pragma once

#include "zmq.hpp"
#include "ImageTask.h"
#include "ModelTask.h"

#include "containers/vector.h"
#include "containers/string.h"
#include "containers/memory.h"

class ProcessorTaskImageRequest;
class ProcessorTaskImageResponse;

namespace resource_task
{
    class ResourceTask
    {
    public:
        ResourceTask(
            const engine::string& ip, 
            int port, 
            const engine::string& id,
            const engine::string& hostId);
        int join();

        zmq::socket_t& socket()
        {
            return m_socket;
        }
        const engine::string& hostId() const
        {
            return m_hostid;
        }
    private:
        engine::string m_id;
        engine::string m_hostid;
        zmq::context_t m_context;
        zmq::socket_t m_socket;
        std::vector<zmq::pollitem_t> m_polls;

        ImageTask m_imageTask;
        ModelTask m_modelTask;

        enum class TaskState
        {
            WaitingForHelo,
            PerformingTask
        };
        TaskState m_state;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_lastCheck;
    };
}
