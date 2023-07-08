#include "engine/network/NetworkCommunication.h"
#include "engine/network/MqMessage.h"
#include "engine/network/Beacon.h"
#include "tools/Debug.h"
#include "platform/network/SocketClient.h"
#include "platform/network/Socket.h"
#include "platform/Uuid.h"

#include "protocols/network/ResourceProtocols.pb.h"

using namespace engine;
using namespace zmq;

#undef ENABLE_RESOURCE_CLIENT_LOGGING

namespace engine
{
    NetworkCommunication::NetworkCommunication(
        OnNetworkResourceDiscovery found,
        OnNetworkResourceLoss lost,
        NetTaskNotifyStarted started,
        NetTaskNotifyFinished finished,
        NetTaskNotifyProgress progress,
        NetTaskNotifyProgressMessage progressMessage,
        NetTaskImageResult imageResult,
        NetTaskModelResult modelResult)
        : m_commMutex{}
        , m_commTasks{}
        , m_uuid{ platform::uuid() }
        , m_found{ found }
        , m_lost{ lost }
        , m_started{ started }
        , m_finished{ finished }
        , m_progress{ progress }
        , m_progressMessage{ progressMessage }
        , m_imageResult{ imageResult }
        , m_modelResult{ modelResult }
        , m_beacon{ engine::make_shared<Beacon>(
            BeaconType::Client,
            m_uuid,
            [this](const BeaconHello& hello) { this->onDiscovery(hello); },
            [this](const BeaconHello& hello) { this->onLoss(hello); }) }
        , m_alive{ true }
        , m_commThread{ [this]() { this->commHeartbeat(); } }
    {
    }

    NetworkCommunication::~NetworkCommunication()
    {
        {
			std::lock_guard<std::mutex> lock(m_commMutex);
            m_alive = false;
        }
        m_commThread.join();
    }

    void NetworkCommunication::onDiscovery(const BeaconHello& beacon)
    {
#ifdef ENABLE_RESOURCE_CLIENT_LOGGING
        LOG_INFO("Found resource client through discovery: %s", beacon.ip().c_str());
#endif
        {
			std::lock_guard<std::mutex> lock(m_commMutex);
            m_commTasks.push(CommTask{ CommTaskType::ClientEntry, beacon });
        }
    }

    void NetworkCommunication::onLoss(const BeaconHello& beacon)
    {
#ifdef ENABLE_RESOURCE_CLIENT_LOGGING
        LOG_INFO("Lost resource client: %s", beacon.ip().c_str());
#endif
        {
			std::lock_guard<std::mutex> lock(m_commMutex);
            m_commTasks.push(CommTask{ CommTaskType::ClientLeave, beacon });
        }
        if (m_lost)
            m_lost(beacon);
    }

    void NetworkCommunication::send(MqMessage&& msg, const BeaconHello& beacon)
    {
		std::lock_guard<std::mutex> lock(m_commMutex);
        m_commTasks.push(CommTask{ CommTaskType::Send, beacon, std::move(msg) });
    }

    vector<char> packageType(HostProcessorMessageType_MessageType type)
    {
        HostProcessorMessageType reqtype;
        reqtype.set_type(type);
        vector<char> typeMessage(reqtype.ByteSizeLong());
        if (typeMessage.size() > 0)
            reqtype.SerializeToArray(&typeMessage[0], static_cast<int>(typeMessage.size()));
        return typeMessage;
    }

    MqMessage msgTo(const string& address, HostProcessorMessageType_MessageType type)
    {
        MqMessage msg;
        msg.emplace_back(address);
        msg.emplace_back("");
        msg.emplace_back(packageType(type));
        return msg;
    }

    uint32_t NetworkCommunication::cores(const BeaconHello& beacon)
    {
        std::lock_guard<std::mutex> lock(m_commMutex);
        for (auto&& client : m_clients)
        {
            if (client.beacon.id() == beacon.id())
            {
                return client.cores;
            }
        }
        return 0;
    }

    void NetworkCommunication::commHeartbeat()
    {
        m_context = engine::make_unique<context_t>(context_t{ 1 });
        m_router = engine::make_unique<socket_t>(socket_t{ *m_context, socket_type::router });
        m_router->setsockopt(ZMQ_IDENTITY, m_uuid.data(), m_uuid.size());
        m_router->setsockopt<int>(ZMQ_ROUTER_MANDATORY, 1);
        m_polls.emplace_back(pollitem_t{ *m_router, 0, ZMQ_POLLIN,  0 });

        bool alive = true;
        while (alive)
        {
            CommTask task;
            bool haveTask = false;
            {
                std::lock_guard<std::mutex> lock(m_commMutex);
                alive = m_alive;
                if (alive && m_commTasks.size() > 0)
                {
                    task = std::move(m_commTasks.front());
                    m_commTasks.pop();
                    haveTask = true;
                }
            }

            if (haveTask)
            {
                switch (task.type)
                {
                    case CommTaskType::ClientEntry:
                    {
                        m_clients.emplace_back(ClientContainer{ 
                            task.beacon, 
                            0,
                            std::chrono::high_resolution_clock::now() });
                        m_router->connect(addressFromIp(task.beacon.ip().c_str()).c_str());

#ifdef ENABLE_RESOURCE_CLIENT_LOGGING
                        LOG("HOST Connecting to TASK DISTRIBUTOR: %s, ID: %s", 
                            addressFromIp(task.beacon.ip().c_str()).c_str(),
                            task.beacon.id().c_str());
#endif

                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        break;
                    }
                    case CommTaskType::ClientLeave:
                    {
                        for (auto client = m_clients.begin(); client != m_clients.end(); ++client)
                        {
                            if ((*client).beacon.id() == task.beacon.id())
                            {
                                m_clients.erase(client);
                                break;
                            }
                        }

                        m_router->disconnect(addressFromIp(task.beacon.ip().c_str()).c_str());
#ifdef ENABLE_RESOURCE_CLIENT_LOGGING
                        LOG("HOST Disconnecting from TASK DISTRIBUTOR: %s", addressFromIp(task.beacon.ip().c_str()).c_str());
#endif
                        break;
                    }
                    case CommTaskType::Send:
                    {
                        task.msg.prepend("");
                        task.msg.prepend(task.beacon.id().c_str());
                        task.msg.send(*m_router);

#ifdef ENABLE_RESOURCE_CLIENT_LOGGING
                        LOG("HOST Sending task to: %s, ID: %s", 
                            addressFromIp(task.beacon.ip().c_str()).c_str(),
                            task.beacon.id().c_str());
#endif
                    }
                }
            }

            {
                auto now = std::chrono::high_resolution_clock::now();
                std::lock_guard<std::mutex> lock(m_commMutex);
                for (auto&& client : m_clients)
                {
                    if (client.cores == 0)
                    {
                        auto durationMs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - client.lastSent).count()) / 1000000.0;
                        if (durationMs > 1000)
                        {
                            auto msg = msgTo(client.beacon.id().c_str(), HostProcessorMessageType::CoreRequest);
                            msg.send(*m_router);
                            client.lastSent = now;
                        }
                    }
                }
            }

            poll(m_polls, 20);

            if (m_polls[0].revents & ZMQ_POLLIN)
            {
                MqMessage msg(*m_router);
                string id = string(&msg.parts()[0][0], msg.parts()[0].size());

                HostProcessorMessageType type;
                type.ParseFromArray(msg.parts()[1].data(), static_cast<int>(msg.parts()[1].size()));

                if (type.type() == HostProcessorMessageType::CoreResponse)
                {
                    HostCoreResponse answer;
                    answer.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));
                    
                    BeaconHello hel;
                    bool foundHel = false;
                    {
                        std::lock_guard<std::mutex> lock(m_commMutex);
                        for (auto&& client : m_clients)
                        {
                            if (engine::string(client.beacon.id().c_str()) == id && client.cores == 0)
                            {
                                client.cores = answer.cores();
                                hel = client.beacon;
                                foundHel = true;
                                break;
                            }
                        }
                    }
                    if (m_found && foundHel)
                        m_found(hel);
                }
                if (type.type() == HostProcessorMessageType::TaskStarted)
                {
                    HostTaskStarted start;
                    start.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));
                    if (m_started)
                        m_started(start.taskid().c_str());
                }
                if (type.type() == HostProcessorMessageType::TaskFinished)
                {
                    HostTaskFinished finished;
                    finished.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));
                    if (m_finished)
                        m_finished(finished.taskid().c_str());
                }
                if (type.type() == HostProcessorMessageType::TaskProgress)
                {
                    HostTaskProgress progress;
                    progress.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));
                    if (m_progress)
                        m_progress(progress.taskid().c_str(), progress.progress());
                }
                if (type.type() == HostProcessorMessageType::TaskProgressMessage)
                {
                    HostTaskProgressMessage progress;
                    progress.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));
                    if (m_progressMessage)
                        m_progressMessage(progress.taskid().c_str(), progress.progress(), progress.message().c_str());
                }
                if (type.type() == HostProcessorMessageType::TaskImageResponse)
                {
                    HostTaskImageResponse resp;
                    resp.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));
                    if (m_imageResult)
                        m_imageResult(resp);
                }
                if (type.type() == HostProcessorMessageType::TaskModelResponse)
                {
                    HostTaskModelResponse resp;
                    resp.ParseFromArray(msg.parts()[2].data(), static_cast<int>(msg.parts()[2].size()));
                    if (m_modelResult)
                        m_modelResult(resp);
                }
            }
        }

    }

    engine::string NetworkCommunication::addressFromIp(const engine::string& ip)
    {
        string address = "tcp://";
        address += ip;
        address += ":12134";
        return address;
    }
}
