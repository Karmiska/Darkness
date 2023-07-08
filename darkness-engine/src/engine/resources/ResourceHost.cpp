#include "engine/resources/ResourceHost.h"
#include "engine/resources/ResourceMessages.h"
#include "engine/network/Beacon.h"
#include "engine/resources/ResourceDropHandler.h"
#include "engine/network/MqMessage.h"
#include "platform/Directory.h"
#include "tools/AssetTools.h"
#include "platform/network/SocketClient.h"
#include "platform/network/Socket.h"
#include "platform/network/PrefixLengthMessageParser.h"
#include "platform/Environment.h"
#include "tools/Debug.h"
#include "tools/Process.h"
#include "tools/PathTools.h"
#include "platform/Uuid.h"
#include "tools/image/Image.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Quaternion.h"

#include "protocols/network/ResourceProtocols.pb.h"

#include <chrono>
#include <thread>
#include "containers/vector.h"
#include <fstream>

//#include "zmq.h"

using namespace platform;
using namespace engine;
using namespace zmq;

namespace engine
{
    ResourceHost::ResourceHost()
        : m_taskAlive{ true }
        , m_taskWorkedOn{ false }
        , m_localResourceProcess{ false }
        , m_netComm {
            // found resource
            [this](const BeaconHello& beacon)
            {
				std::lock_guard<std::mutex> lock(this->m_clientMutex);
                this->m_knownClients.emplace_back(ClientContainer{
                    beacon,
                    this->m_netComm.cores(beacon),
                    {} });
            },
            // lost resource
            [this](const BeaconHello& beacon)
            {
				std::lock_guard<std::mutex> lock(this->m_clientMutex);
                for (auto client = this->m_knownClients.begin(); client != this->m_knownClients.end(); ++client)
                {
                    if ((*client).client.id() == beacon.id())
                    {
                        this->m_knownClients.erase(client);
                        break;
                    }
                }
            },
            // task started 1.
            [this](const string& id)
            {
				std::lock_guard<std::mutex> lock(m_tasksListMutex);
                ASSERT(m_tasks.find(id) != m_tasks.end(), "Got task started for task that does not exist");
                if (m_workItem.onStarted)
                    m_workItem.onStarted(m_tasks[id]->item);
            },
            // task finished 4.
            [this](const string& id)
            {
                {
					std::lock_guard<std::mutex> lock(this->m_clientMutex);
                    bool handled = false;
                    for (auto&& client : this->m_knownClients)
                    {
                        for (auto activeTask = client.tasksActive.begin(); activeTask != client.tasksActive.end(); ++activeTask)
                        {
                            if ((*activeTask) == id)
                            {
                                client.tasksActive.erase(activeTask);
                                handled = true;
                                break;
                            }
                        }
                        if (handled)
                            break;
                    }
                }

				std::lock_guard<std::mutex> lock(m_tasksListMutex);
                ASSERT(m_tasks.find(id) != m_tasks.end(), "Got finish signal for task that does not exist");
                if (m_workItem.onFinished)
                    m_workItem.onFinished(m_tasks[id]->item);

                for (auto item = this->m_visibleItems.begin(); item != this->m_visibleItems.end(); ++item)
                {
                    if ((*item).absoluteSourceFilepath == m_tasks[id]->item.absoluteSourceFilepath)
                    {
                        this->m_visibleItems.erase(item);
                        break;
                    }
                }

                m_tasks.erase(id);
            },
            // task progress 2.
            [this](const string& id, float progress)
            {
				std::lock_guard<std::mutex> lock(m_tasksListMutex);
                ASSERT(m_tasks.find(id) != m_tasks.end(), "Got progress for task that does not exist");
                if (m_workItem.onProgress)
                    m_workItem.onProgress(m_tasks[id]->item, progress);
            },
            // task progress 2.
                [this](const string& id, float progress, const string& message)
            {
				std::lock_guard<std::mutex> lock(m_tasksListMutex);
                ASSERT(m_tasks.find(id) != m_tasks.end(), "Got progress for task that does not exist");
                if (m_workItem.onProgressMessage)
                    m_workItem.onProgressMessage(m_tasks[id]->item, progress, message);
            },
            // task results 3.
            [this](HostTaskImageResponse response)
            {
				std::lock_guard<std::mutex> lock(m_tasksListMutex);
                ASSERT(m_tasks.find(engine::string(response.taskid().c_str())) != m_tasks.end(), "Got task results for task that does not exist");
                ProcessingTask& task = *m_tasks[response.taskid().c_str()];
                //writeFile(task.item.absoluteProcessedFilepath, response.data());

                engine::Directory dir(pathExtractFolder(task.item.absoluteProcessedFilepath));
                if (!dir.exists())
                    dir.create();

                auto image = engine::image::Image::createImage(
                    task.item.absoluteProcessedFilepath,
                    image::ImageType::DDS, 
                    static_cast<Format>(response.format()),
                    response.width(),
                    response.height(),
                    1, 
                    response.mips());
                image->save(response.data().data(), response.data().size());
            },
            // task results 3.
            [this](HostTaskModelResponse response)
            {
				std::lock_guard<std::mutex> lock(m_tasksListMutex);
                ASSERT(m_tasks.find(engine::string(response.taskid().c_str())) != m_tasks.end(), "Got task results for task that does not exist");
                ProcessingTask& task = *m_tasks[response.taskid().c_str()];
                //writeFile(task.item.absoluteProcessedFilepath, response.data());

                engine::Directory dir(pathExtractFolder(task.item.absoluteProcessedFilepath));
                if (!dir.exists())
                    dir.create();

				std::ofstream mesh;
                mesh.open(task.item.absoluteProcessedFilepath.c_str(), std::ios::out | std::ios::binary);
                if (mesh.is_open())
                {
                    mesh.write(response.modeldata().data(), response.modeldata().size());
                    mesh.close();
                }

                auto prefabPath = pathReplaceExtension(task.item.absoluteContentFilepath, "prefab");
				std::ofstream prefab;
                prefab.open(prefabPath.c_str(), std::ios::out | std::ios::binary);
                if (prefab.is_open())
                {
                    prefab.write(response.prefabdata().data(), response.prefabdata().size());
                    prefab.close();
                }
            }
    }
    {
        m_taskThread = std::move(engine::unique_ptr<std::thread, std::function<void(std::thread*)>>(new std::thread(
            // task
            [this]()
            {
                this->taskHandler();
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

        m_delayedLocalResourceClientStart = std::move(engine::unique_ptr<std::thread, std::function<void(std::thread*)>>(new std::thread(
            // task
            [this]()
            {
                this->delayedStart();
            }),

            // deleter
            [&](std::thread* ptr) {
                ptr->join();
                delete ptr;
        }));
    }

    void ResourceHost::delayedStart()
    {
		std::this_thread::sleep_for(std::chrono::milliseconds(4000));

        bool weHaveResourceProcessor = false;
        {
			std::lock_guard<std::mutex> lock(m_clientMutex);
            weHaveResourceProcessor = m_knownClients.size() > 0;
        }

        if (!weHaveResourceProcessor)
        {
            m_localResourceProcess = true;
            startLocalResourceClient();
        }
    }

    void ResourceHost::startLocalResourceClient()
    {
        auto exeLocation = engine::getExecutableDirectory();
        auto clientPath = pathClean(pathJoin(pathExtractFolder(exeLocation), ResourceClientPath));
        auto clientWorkingDirPath = pathClean(pathJoin(pathExtractFolder(exeLocation), ResourceClientWorkingPath));
        if (!pathExists(clientPath))
        {
            clientPath = pathClean(pathJoin(pathExtractFolder(exeLocation), ResourceClientFallbackPath));
            clientWorkingDirPath = pathClean(pathJoin(pathExtractFolder(exeLocation), ResourceClientFallbackWorkingPath));
        }
        ASSERT(
            pathExists(clientPath),
            "Resource Client executable has not been built. Looking from %s",
            clientPath.c_str());

        m_localResourceProcessId = platform::uuid();

        string args = "-headless ";
        args += "--identity=";
        args += m_localResourceProcessId;

        m_resourceClientProcess = engine::make_shared<Process>(
            clientPath,
            args,
            clientWorkingDirPath,
            [this](const string& message)
        {
            LOG_PURE("%s", message.c_str());
        });
    }

    void ResourceHost::stopLocalResourceClient()
    {
        HostProcessorMessageType type;
        type.set_type(HostProcessorMessageType::ShutdownProcessor);
        vector<char> type_message(type.ByteSizeLong());
        if (type_message.size() > 0)
            type.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

        MqMessage msg;
        msg.emplace_back(move(type_message));

        BeaconHello beacon;
        beacon.set_ip(Socket::localIp().c_str());
        beacon.set_id(m_localResourceProcessId.c_str());
        beacon.set_type(BeaconHello::Server);
        beacon.set_action(BeaconHello::Alive);

        m_netComm.send(std::move(msg), beacon);
    }

    ResourceHost::~ResourceHost()
    {
        bool weHaveLocalResourceProcess = false;
        {
			std::lock_guard<std::mutex> lock(m_clientMutex);
            weHaveLocalResourceProcess = m_localResourceProcess;
        }

        if (weHaveLocalResourceProcess)
        {
            stopLocalResourceClient();
        }

        m_taskThread = nullptr;
    }

    void ResourceHost::taskHandler()
    {
        bool taskQueueEmpty = false;
        bool taskAlive = true;
        while (taskAlive || !taskQueueEmpty || m_taskWorkedOn)
        {
            {
				std::lock_guard<std::mutex> lock(m_taskMutex);
                taskAlive = m_taskAlive;
            }

            taskQueueEmpty = true;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
            {
				std::lock_guard<std::mutex> lock(m_taskMutex);
                taskQueueEmpty = m_taskQueue.size() == 0;
                if (!m_taskWorkedOn && m_taskQueue.size() > 0)
                {
                    m_workItem = std::move(m_taskQueue.front());
                    m_taskQueue.pop_front();
                    m_taskWorkedOn = true;

                    m_taskWorkerThread = move(engine::unique_ptr<std::thread, std::function<void(std::thread*)>>(new std::thread(
                        // task
                        [this]()
                        {
                            this->taskWorkerHandler();
                        }),

                        // deleter
                        [&](std::thread* ptr) {
                            ptr->join();
                            delete ptr;
                    }));
                }
            }
            {
				std::lock_guard<std::mutex> lock(m_taskMutex);
                if (!m_taskWorkedOn && m_taskWorkerThread)
                {
                    m_taskWorkerThread = nullptr;
                }
            }

            forceItemVisibility();
        }
    }

    void ResourceHost::forceItemVisibility()
    {
		std::lock_guard<std::mutex> lock(m_taskMutex);
		std::lock_guard<std::mutex> tasklock(m_tasksListMutex);
        if (m_visibleItems.size() >= FakeStartMaxSize)
            return;

        for (auto& task : m_tasks)
        {
            // do we have this already ?
            bool found = false;
            for (auto vis : m_visibleItems)
            {
                if (vis.absoluteSourceFilepath == task.second->item.absoluteSourceFilepath)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                m_visibleItems.emplace_back(task.second->item);
                if(m_workItem.onStarted)
                    m_workItem.onStarted(task.second->item);
            }

            if (m_visibleItems.size() >= FakeStartMaxSize)
                return;
        }

        // still here? okay we need to fetch more items. let's try from the workitem
        // it's exhausted from back to front
        for (auto item = m_workItem.items.rbegin(); item != m_workItem.items.rend(); ++item)
        {
            bool found = false;
            for (auto vis : m_visibleItems)
            {
                if (vis.absoluteSourceFilepath == (*item).absoluteSourceFilepath)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                m_visibleItems.emplace_back(*item);
                if (m_workItem.onStarted)
                    m_workItem.onStarted(*item);
            }

            if (m_visibleItems.size() >= FakeStartMaxSize)
                return;
        }

        // really? well okay then. start digging in to queue
        // queue is exhausted from front to back

        for (auto&& item = m_taskQueue.begin(); item != m_taskQueue.end(); ++item)
        {
            for (auto subitem = (*item).items.rbegin(); subitem != (*item).items.rend(); ++subitem)
            {
                bool found = false;
                for (auto vis : m_visibleItems)
                {
                    if (vis.absoluteSourceFilepath == (*subitem).absoluteSourceFilepath)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    m_visibleItems.emplace_back(*subitem);
                    if (m_workItem.onStarted)
                        m_workItem.onStarted(*subitem);
                }

                if (m_visibleItems.size() >= FakeStartMaxSize)
                    return;
            }
        }
    }

    void ResourceHost::taskWorkerHandler()
    {
        struct WorkItem
        {
            bool valid;
            ProcessResourceItem item;
        };

        auto hasTask = [&]()->WorkItem
        {
            WorkItem result{ false,{} };
            {
				std::lock_guard<std::mutex> lock(m_taskMutex);
                if (m_workItem.items.size() > 0)
                {
                    result.item = std::move(m_workItem.items.back());
                    m_workItem.items.pop_back();
                    result.valid = true;
                }
            }
            return result;
        };

        auto item = hasTask();
        while (item.valid)
        {
            // process item
            {
                string taskId = platform::uuid();
                auto newTask = engine::make_unique<ProcessingTask>();
                newTask->item = item.item;
                newTask->id = taskId;
                // THIS THREAD GETS BLOCKED HERE (BY DESIGN)
                auto b = getAvailableClient(taskId);
                {
					std::lock_guard<std::mutex> lock(m_tasksListMutex);
                    LOG("Processing item: %s", item.item.absoluteSourceFilepath.c_str());
                    m_tasks[taskId] = move(newTask);
                    sendTask(b, m_tasks[taskId].get());
                }
            }
            // get next item
            item = hasTask();
        }

        auto tasksLeft = [&]()->bool
        {
			std::lock_guard<std::mutex> lock(m_tasksListMutex);
            return m_tasks.size() > 0;
        };
        // THIS THREAD GETS BLOCKED HERE (BY DESIGN)
        while (tasksLeft())
        {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        {
			std::lock_guard<std::mutex> lock(m_taskMutex);
            m_taskWorkedOn = false;
        }
    }

    void ResourceHost::sendTask(BeaconHello& beacon, ProcessingTask* task)
    {
        auto srcData = readFile(task->item.absoluteSourceFilepath);

        if (engine::isImageFormat(task->item.absoluteSourceFilepath))
        {
            HostProcessorMessageType type;
            type.set_type(HostProcessorMessageType::TaskImageRequest);
            vector<char> type_message(type.ByteSizeLong());
            if (type_message.size() > 0)
                type.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

            HostTaskImageRequest req;
            req.set_taskid(task->id.c_str());
            req.set_extension(pathExtractExtension(task->item.absoluteSourceFilepath).c_str());
            req.set_format(task->item.encodingFormat);
            req.set_generatemips(task->item.generateMips);
			req.set_flipnormal(task->item.flipNormal);
			req.set_alphaclipped(task->item.alphaClipped);
            req.set_data(srcData.data(), srcData.size());
            vector<char> message(req.ByteSizeLong());
            if (message.size() > 0)
                req.SerializeToArray(&message[0], static_cast<int>(message.size()));

            MqMessage msg;
            msg.emplace_back(move(type_message));
            msg.emplace_back(move(message));
            m_netComm.send(std::move(msg), beacon);
        }
        else if (engine::isModelFormat(task->item.absoluteSourceFilepath))
        {
            HostProcessorMessageType type;
            type.set_type(HostProcessorMessageType::TaskModelRequest);
            vector<char> type_message(type.ByteSizeLong());
            if (type_message.size() > 0)
                type.SerializeToArray(&type_message[0], static_cast<int>(type_message.size()));

            // placeholder
            engine::Vector3f scale{1.0f, 1.0f, 1.0f};
            engine::Quaternionf rotation = Quaternionf::fromMatrix(Matrix4f::rotation(0.0f, 0.0f, 0.0f));

            auto filename = pathExtractFilename(task->item.absoluteSourceFilepath);
            auto ext = pathExtractExtension(filename);
            auto plainFilename = filename.substr(0, filename.length() - ext.length() - 1);

            HostTaskModelRequest req;
            req.set_taskid(task->id.c_str());
            req.set_modeltargetpath(task->item.absoluteProcessedFilepath.c_str());
            req.set_assetname(plainFilename.c_str());
            req.set_scalex(scale.x);
            req.set_scaley(scale.y);
            req.set_scalez(scale.z);
            req.set_rotationx(rotation.x);
            req.set_rotationy(rotation.y);
            req.set_rotationz(rotation.z);
            req.set_rotationw(rotation.w);
            req.set_data(srcData.data(), srcData.size());
            vector<char> message(req.ByteSizeLong());
            if (message.size() > 0)
                req.SerializeToArray(&message[0], static_cast<int>(message.size()));

            MqMessage msg;
            msg.emplace_back(move(type_message));
            msg.emplace_back(move(message));
            m_netComm.send(std::move(msg), beacon);
        }
    }

    BeaconHello ResourceHost::getAvailableClient(const engine::string& taskId)
    {
        bool foundClient = false;
        BeaconHello beacon;
        while (!foundClient)
        {
            {
				std::lock_guard<std::mutex> lock(m_clientMutex);
                for (auto&& client : m_knownClients)
                {
                    if (client.tasksActive.size() == 0)
                    {
                        beacon = client.client;
                        client.tasksActive.emplace_back(taskId);
                        foundClient = true;
                        break;
                    }
                }
                /*int bestClientIndex = -1;
                int leastCurrentWork = numeric_limits<int>::max();
                for (int i = 0; i < m_knownClients.size(); ++i)
                {
                    if (m_knownClients[i].tasksActive.size() < leastCurrentWork &&
                        m_knownClients[i].cores - m_knownClients[i].tasksActive.size() > 0)
                    {
                        bestClientIndex = i;
                        leastCurrentWork = m_knownClients[i].tasksActive.size();
                    }
                }

                if (bestClientIndex != -1)
                {
                    beacon = m_knownClients[bestClientIndex].client;
                    m_knownClients[bestClientIndex].tasksActive.emplace_back(taskId);
                    foundClient = true;
                }*/
            }
            if(!foundClient)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return beacon;
    }

    vector<char> ResourceHost::readFile(const string& path)
    {
        vector<char> srcData;
		std::ifstream file;
        // TODO: THIS SHOULD BE CONTENT FILEPATH NOT SOURCE, BUT FOR TESTING THIS WILL DO
        file.open(path.c_str(), std::ios::binary);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
            size_t size = static_cast<size_t>(file.tellg());
            file.seekg(0, std::ios::beg);
            srcData.resize(size);
            file.read(&srcData[0], size);
            file.close();
        }
        return srcData;
    }

    void ResourceHost::processResources(const ProcessResourcePackage& package)
    {
        if (package.models.size() > 0 || package.images.size() > 0)
        {
            std::lock_guard<std::mutex> lock(m_taskMutex);
            m_taskQueue.push_back(ProcessingItem{
                package.images,
                package.onStarted,
                package.onFinished,
                package.onProgress,
                package.onProgressMessage });

            m_taskQueue.push_back(ProcessingItem{
                package.models,
                package.onStarted,
                package.onFinished,
                package.onProgress,
                package.onProgressMessage });
        }
    }

}
