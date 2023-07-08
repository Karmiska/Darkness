#pragma once

#include "zmq.hpp"
#include "engine/network/MqMessage.h"
#include "engine/network/Beacon.h"
#include "containers/memory.h"
#include "containers/vector.h"
#include <mutex>
#include <thread>
#include "containers/string.h"
#include <queue>

class BeaconHello;
class HostTaskImageResponse;
class HostTaskModelResponse;

namespace engine
{
    class Beacon;

    using NetTaskNotifyStarted = std::function<void(const engine::string&)>;
    using NetTaskNotifyFinished = std::function<void(const engine::string&)>;
    using NetTaskNotifyProgress = std::function<void(const engine::string&, float)>;
    using NetTaskNotifyProgressMessage = std::function<void(const engine::string&, float, const engine::string&)>;
    using NetTaskImageResult = std::function<void(HostTaskImageResponse)>;
    using NetTaskModelResult = std::function<void(HostTaskModelResponse)>;

    class NetworkCommunication
    {
    public:
        NetworkCommunication(
            OnNetworkResourceDiscovery found,
            OnNetworkResourceLoss lost,
            NetTaskNotifyStarted started,
            NetTaskNotifyFinished finished,
            NetTaskNotifyProgress progress,
            NetTaskNotifyProgressMessage progressMessage,
            NetTaskImageResult imageResult,
            NetTaskModelResult modelResult);
        ~NetworkCommunication();

        uint32_t cores(const BeaconHello& beacon);
        void send(MqMessage&& msg, const BeaconHello& beacon);

    private:
        std::mutex m_commMutex;
        enum class CommTaskType
        {
            ClientEntry,
            ClientLeave,
            Send
        };
        struct CommTask
        {
            CommTaskType type;
            BeaconHello beacon;
            MqMessage msg;
        };
        std::queue<CommTask> m_commTasks;

        engine::string m_uuid;
        OnNetworkResourceDiscovery m_found;
        OnNetworkResourceLoss m_lost;
        NetTaskNotifyStarted m_started;
        NetTaskNotifyFinished m_finished;
        NetTaskNotifyProgress m_progress;
        NetTaskNotifyProgressMessage m_progressMessage;
        NetTaskImageResult m_imageResult;
        NetTaskModelResult m_modelResult;
        engine::shared_ptr<Beacon> m_beacon;
        void onDiscovery(const BeaconHello& beacon);
        void onLoss(const BeaconHello& beacon);

        volatile bool m_alive;
        engine::unique_ptr<zmq::context_t> m_context;
        engine::unique_ptr<zmq::socket_t> m_router;
        engine::vector<zmq::pollitem_t> m_polls;

        std::thread m_commThread;
        void commHeartbeat();

        struct ClientContainer
        {
            BeaconHello beacon;
            unsigned int cores;
            std::chrono::time_point<std::chrono::high_resolution_clock> lastSent;
        };
        engine::vector<ClientContainer> m_clients;

        engine::string addressFromIp(const engine::string& ip);
    };
}
