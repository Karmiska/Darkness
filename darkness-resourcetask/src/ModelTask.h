#pragma once

#include "containers/string.h"
#include <functional>
#include "containers/vector.h"

namespace engine
{
    struct ModelCpu;
}

namespace zmq
{
    class socket_t;
}

class ProcessorTaskModelRequest;
class ProcessorTaskModelResponse;

namespace resource_task
{
    class ModelTask
    {
    public:
        struct FinishedData
        {
            engine::vector<char> modelData;
            engine::vector<char> prefabData;
        };

        ProcessorTaskModelResponse process(
            ProcessorTaskModelRequest& request,
            const engine::string& hostId,
            zmq::socket_t* socket);

        void process(
            const engine::string& srcFile,
            const engine::string& dstFile);

    private:

        FinishedData privateProcess(
            const char* buffer,
            size_t bytes,
            const engine::string& taskId,
            const engine::string& hostId,
            const engine::string& assetName,
            const engine::string& modelTargetPath,
            float scaleX,
            float scaleY,
            float scaleZ,
            float rotationX,
            float rotationY,
            float rotationZ,
            float rotationW,
            zmq::socket_t* socket,
            std::function<void(
                const engine::string&,
                const engine::string&,
                zmq::socket_t*,
                float,
                const engine::string&
                )> onUpdateProgress);
    };
}
