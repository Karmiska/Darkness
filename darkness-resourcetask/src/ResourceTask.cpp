#include "ResourceTask.h"
#include "platform/network/SocketClient.h"
#include "platform/network/Socket.h"
#include "engine/resources/ResourceMessages.h"
#include "engine/network/MqMessage.h"
#include "tools/Debug.h"
#include <chrono>
#include <thread>

#include "protocols/network/ResourceProtocols.pb.h"

using namespace platform;
using namespace engine;
using namespace zmq;

namespace resource_task
{
    ResourceTask::ResourceTask(
        const engine::string& ip, 
        int port, 
        const string& id,
        const string& hostId)
        : m_id{ id }
        , m_hostid{ hostId }
        , m_context{ 1 }
        , m_socket{ m_context, socket_type::router }
        , m_polls{ { m_socket, 0, ZMQ_POLLIN,  0 } }
        , m_state{ TaskState::WaitingForHelo }
        , m_lastCheck{ std::chrono::high_resolution_clock::now() }
    {
        /*LOG("ResourceTask started. Task ID: %s, Host IP: %s, Host PORT: %s, Host ID: %s", 
            id.c_str(),
            ip.c_str(),
            to_string(port).c_str(),
            hostId.c_str());*/
        //std::this_thread::sleep_for(std::chrono::milliseconds(60000));

        m_socket.setsockopt(ZMQ_IDENTITY, m_id.data(), m_id.size());
        m_socket.setsockopt<int>(ZMQ_ROUTER_MANDATORY, 1);
        string connAddress = "tcp://";
        connAddress += ip;
        connAddress += ":";
        connAddress += std::to_string(port).c_str();
        m_socket.connect(connAddress.c_str());
        LOG("Connecting to TASK DISTRIBUTOR at: %s, ID: %s",
            connAddress.c_str(),
            hostId.c_str());
    }

    int ResourceTask::join()
    {
        while (true)
        {
            poll(m_polls, 20);

            if (m_polls[0].revents & ZMQ_POLLIN)
            {
                m_state = TaskState::PerformingTask;

                MqMessage msg(m_socket);
                string id = string(&msg.parts()[0][0], msg.parts()[0].size());

                ProcessorTaskMessageType type;
                type.ParseFromArray(msg.parts()[1].data(), static_cast<int>(msg.parts()[1].size()));

                switch (type.type())
                {
                    case ProcessorTaskMessageType::TaskRequest:
                    {
                        ProcessorTaskRequest requesttype;
                        requesttype.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));

						engine::vector<char> responseData;

                        switch (requesttype.type())
                        {
                            case ProcessorTaskRequest::Image:
                            {
                                ProcessorTaskImageRequest imageReq;
                                imageReq.ParseFromArray(msg.parts()[3].data(), static_cast<int>(msg.parts()[3].size()));
                                LOG("Task was asked to perform Image encoding: %s", imageReq.taskid().c_str());
                                auto response = m_imageTask.process(imageReq, m_hostid, &m_socket);
                                responseData.resize(response.ByteSizeLong());
                                if(responseData.size() > 0)
                                    response.SerializeToArray(&responseData[0], static_cast<int>(responseData.size()));
                                break;
                            }
                            case ProcessorTaskRequest::Model:
                            {
                                ProcessorTaskModelRequest modelReq;
                                modelReq.ParseFromArray(msg.parts()[3].data(), static_cast<int>(msg.parts()[3].size()));
                                LOG("Task was asked to perform Model encoding: %i", modelReq.taskid().c_str());
                                auto response = m_modelTask.process(modelReq, m_hostid, &m_socket);

                                responseData.resize(response.ByteSizeLong());
                                if (responseData.size() > 0)
                                    response.SerializeToArray(&responseData[0], static_cast<int>(responseData.size()));
                                break;
                            }
                        }

                        ProcessorTaskMessageType resulttype;
                        resulttype.set_type(ProcessorTaskMessageType::TaskResponse);
						engine::vector<char> type_message(resulttype.ByteSizeLong());
                        if (type_message.size() > 0)
                            resulttype.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

                        MqMessage responseMsg;
                        responseMsg.emplace_back(id);
                        responseMsg.emplace_back("");
                        responseMsg.emplace_back(std::move(type_message));
                        responseMsg.emplace_back(std::move(msg.parts()[2]));
                        responseMsg.emplace_back(std::move(responseData));
                        responseMsg.send(m_socket);

                        break;
                    }
                    case ProcessorTaskMessageType::DoneRequest:
                    {
                        ProcessorTaskMessageType resulttype;
                        resulttype.set_type(ProcessorTaskMessageType::DoneResponse);
						engine::vector<char> type_message(resulttype.ByteSizeLong());
                        if (type_message.size() > 0)
                            resulttype.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

                        MqMessage responseMsg;
                        responseMsg.emplace_back(id);
                        responseMsg.emplace_back("");
                        responseMsg.emplace_back(std::move(type_message));
                        responseMsg.send(m_socket);
                        return 0;
                    }
                }
                //EncodeTaskHeloAnswer
            }

            if (m_state == TaskState::WaitingForHelo)
            {
                auto now = std::chrono::high_resolution_clock::now();
                auto durationMs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastCheck).count()) / 1000000.0;

                if (durationMs > 300)
                {
                    m_lastCheck = now;
                    ProcessorTaskMessageType type;
                    type.set_type(ProcessorTaskMessageType::Helo);
					engine::vector<char> type_message(type.ByteSizeLong());
                    if (type_message.size() > 0)
                        type.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

                    MqMessage msg;
                    msg.emplace_back(m_hostid);
                    msg.emplace_back("");
                    msg.emplace_back(move(type_message));
                    msg.send(m_socket);

                    LOG("Sent TASK helo to ID: %s", m_hostid.c_str());
                }
            }
        }
        LOG("Task exiting");
        return 0;
    }

}
