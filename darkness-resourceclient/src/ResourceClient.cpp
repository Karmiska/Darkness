#include "ResourceClient.h"
#include "platform/network/SocketServer.h"
#include "platform/network/Socket.h"
#include "tools/PathTools.h"
#include "engine/resources/ResourceMessages.h"
#include "engine/resources/ResourceDiscoveryServer.h"
#include "platform/network/PrefixLengthMessageParser.h"
#include "tools/Debug.h"
#include "tools/ProcessorInfo.h"
#include "tools/Process.h"


using namespace platform;
using namespace engine;

namespace resource_client_old
{
    ResourceClient::ResourceClient()
        : m_server{ engine::make_shared<SocketServer>(2134, false, false,
            [](engine::shared_ptr<Socket> socket) { /*LOG("Connection from: %s", socket->ip().c_str());*/ },
            [](engine::shared_ptr<Socket> socket) { /*LOG("Client %s disconnected", socket->ip().c_str());*/ },
            [this](engine::shared_ptr<Socket> socket, const NetworkMessage& message)
            {
                this->onMessage(socket, message);
            })}
        , m_serverToHost{ engine::make_shared<SocketServer>(2135, false, false,
            [](engine::shared_ptr<Socket> socket) { /*LOG("Connection from: %s", socket->ip().c_str());*/ },
            [](engine::shared_ptr<Socket> socket) { /*LOG("Client %s disconnected", socket->ip().c_str());*/ },
            [this](engine::shared_ptr<Socket> socket, const NetworkMessage& message)
            {
                this->onMessageFromHost(socket, message);
            }) }
            , m_discoveryServer{ engine::make_shared<ResourceDiscoveryServer>([](const engine::string&) {}) }
        , m_alive{ true }
        , m_masterSocket{ nullptr }
        , m_magicNumber{ 0 }
        , m_localIp{ Socket::localIp() }
    {

    }

    void ResourceClient::broadcastClientStartup()
    {
        // create client socket
    }

    int ResourceClient::runService()
    {
        // create server to wait for tasks
        bool alive = m_alive;
        while (alive)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                alive = m_alive;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        return 0;
    }

    static int startedProcessCount = 0;
    static int startedTaskCount = 0;
    static int endedTaskCount = 0;

    void ResourceClient::onMessageFromHost(engine::shared_ptr<platform::Socket> socket, const platform::NetworkMessage& message)
    {
        ASSERT(message.bytes >= sizeof(ResourceClientMessage), "Malformed Resource client message");
        ResourceClientMessage msg;
        memcpy(&msg, message.buffer, sizeof(ResourceClientMessage));
        switch (msg)
        {
            case ResourceClientMessage::TestHelo:
            {
                LOG_INFO("Got Helo from Host");
                engine::vector<char> buffer(sizeof(ResourceClientMessage));
                msg = ResourceClientMessage::TestEhlo;
                memcpy(&buffer[0], &msg, sizeof(ResourceClientMessage));
                m_serverToHost->sendMessage(socket, { buffer });
                break;
            }
            case ResourceClientMessage::GetCores:
            {
                // we need the mast puppeteer to return stuff from tasks
                m_masterSocket = socket;

                auto processorInfo = getProcessorInfo();
                engine::vector<char> buffer(sizeof(ResourceClientMessage) + sizeof(uint32_t));
                msg = ResourceClientMessage::CoresAnswer;
                memcpy(&buffer[0], &msg, sizeof(ResourceClientMessage));
                memcpy(&buffer[sizeof(ResourceClientMessage)], &processorInfo.logicalProcessorCount, sizeof(uint32_t));
                m_serverToHost->sendMessage(socket, { buffer });
                break;
            }
            case ResourceClientMessage::ShutdownResourceClient:
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_alive = false;
                return;
            }

            // we need to pass messages to ResourceTasks
            case ResourceClientMessage::ImageEncodingTask:
            {
                // launch a client task
                m_tasks.emplace_back(ResourceClient::ResourceTaskContainer());
                ResourceClient::ResourceTaskContainer& container = m_tasks.back();

                container.magicNumber = m_magicNumber;
                container.encodingTaskData.resize(message.bytes);
                memcpy(&container.encodingTaskData[0], message.buffer, message.bytes);

                LOG("STARTING startedProcessCount: %i, startedTaskCount: %i, endedTaskCount: %i",
                    startedProcessCount, startedTaskCount, endedTaskCount);

                auto taskPath = "";
                if (engine::pathExists(ResourceTaskPath1))
                    taskPath = ResourceTaskPath1;
                else if (engine::pathExists(ResourceTaskPath2))
                    taskPath = ResourceTaskPath2;

                container.taskProcess = engine::make_shared<engine::Process>(
                    taskPath,
                    ResourceTaskArgumentsStart + m_localIp + ResourceTaskArgumentsStop + std::to_string(m_magicNumber).c_str(),
                    ResourceTaskWorkingDir,
                    [](const engine::string& message)
                {
                    LOG_ERROR("%s", message.c_str());
                }
                );

                ++startedProcessCount;

                LOG("STARTED startedProcessCount: %i, startedTaskCount: %i, endedTaskCount: %i",
                    startedProcessCount, startedTaskCount, endedTaskCount);

                uint64_t taskId;
                memcpy(&taskId, &container.encodingTaskData[sizeof(ResourceClientMessage)], sizeof(uint64_t));

                //LOG_WARNING("LAUNCHED TASK PROCESS with MAGIC: %u, TaskID: %u", container.magicNumber, static_cast<uint32_t>(taskId));

                engine::vector<char> buffer(sizeof(ResourceClientMessage));
                msg = ResourceClientMessage::TaskConfirm;
                memcpy(&buffer[0], &msg, sizeof(ResourceClientMessage));
                m_serverToHost->sendMessage(socket, { buffer });

                ++m_magicNumber;
                break;
            }
            default:
                ASSERT(false, "Invalid message");
        }
    }

    void ResourceClient::onMessage(engine::shared_ptr<platform::Socket> socket, const NetworkMessage& message)
    {
        ASSERT(message.bytes >= sizeof(ResourceClientMessage), "Malformed Resource client message");
        ResourceClientMessage msg;
        memcpy(&msg, message.buffer, sizeof(ResourceClientMessage));
        switch (msg)
        {
            case ResourceClientMessage::TaskHelo:
            {
                uint32_t magic;
                memcpy(&magic, message.buffer + sizeof(ResourceClientMessage), sizeof(uint32_t));
                for (auto&& task : m_tasks)
                {
                    if (task.magicNumber == magic)
                    {
                        // sending image encoding task
                        uint64_t taskId;
                        memcpy(&taskId, &task.encodingTaskData[sizeof(ResourceClientMessage)], sizeof(uint64_t));

                        //LOG_WARNING("SENDING ENCODING TASK with MAGIC: %u, TaskID: %u", task.magicNumber, static_cast<uint32_t>(taskId));
                        task.socket = socket;
                        m_server->sendMessage(socket, { task.encodingTaskData });
                    }
                }
                break;
            }

            case ResourceClientMessage::ImageEncodingStart:
            {
                ++startedTaskCount;
                
                LOG("startedProcessCount: %i, startedTaskCount: %i, endedTaskCount: %i",
                    startedProcessCount, startedTaskCount, endedTaskCount);

                LOG("ImageEncoding start");
                m_serverToHost->sendMessage(m_masterSocket, message);
                break;
            }
            case ResourceClientMessage::ImageEncodingProgress:
            {
                //LOG("ImageEncoding progress");
                m_serverToHost->sendMessage(m_masterSocket, message);
                break;
            }
            case ResourceClientMessage::ImageEncodingDone:
            {
                ++endedTaskCount;

                LOG("startedProcessCount: %i, startedTaskCount: %i, endedTaskCount: %i",
                    startedProcessCount, startedTaskCount, endedTaskCount);

                LOG("ImageEncoding done");
                m_serverToHost->sendMessage(m_masterSocket, message);

                ResourceClientMessage recmsg = ResourceClientMessage::TaskResultsReceived;
                engine::vector<char> data(sizeof(ResourceClientMessage));
                memcpy(&data[0], &recmsg, sizeof(ResourceClientMessage));
                m_server->sendMessage(socket, data);
                break;
            }
            default:
                ASSERT(false, "Invalid message");
        }
    }
}
