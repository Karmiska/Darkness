#include "ZmqResourceTask.h"
#include "zmsg.hpp"
#include "Compressonator.h"
#include "engine/resources/ResourceMessages.h"

using namespace zmq;

namespace resource_task_back
{
    ResourceTask::ResourceTask(const engine::string& ip, int port, uint32_t magicNumber)
        : m_context{ 1 }
        , m_socket{ m_context, socket_type::req }
    {
        intptr_t id = static_cast<intptr_t>(magicNumber);
        engine::string identity = s_set_id(m_socket, id).c_str();
        auto connectTarget = engine::string("tcp://") + ip + engine::string(":") + to_string(port).c_str();
        m_socket.connect(connectTarget.c_str());

        s_send(m_socket, "READY");
    }

    int ResourceTask::join()
    {
        while (!m_taskDone)
        {
            zmsg zm(m_socket);

            engine::EncodingTask task;
            performTask(task);

            zm.send(m_socket);
        }
        return 0;
    }

    void ResourceTask::performTask(const engine::EncodingTask& task)
    {
    }
}
