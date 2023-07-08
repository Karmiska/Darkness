#pragma once

#include "containers/string.h"

class ProcessorTaskImageResponse;
class ProcessorTaskImageRequest;

namespace zmq
{
    class socket_t;
}

namespace resource_task
{
    namespace internals
    {
        class CompressonatorInitializer
        {
        public:
            CompressonatorInitializer();
            ~CompressonatorInitializer();
        };
    }

    class ImageTask
    {
    public:
        ProcessorTaskImageResponse process(
            ProcessorTaskImageRequest& request,
            const engine::string& hostId,
            zmq::socket_t* socket);

        void process(
            const engine::string& srcFile,
            const engine::string& dstFile);

    private:
        ProcessorTaskImageResponse privateProcess(
            const engine::string& taskId,
            int enginepackedformat,
            int width,
            int height,
            int targetcmbcformat,
            int stride,
            int sourcecmformat,
            const char* data,
            size_t bytes,
            const engine::string& hostId,
            zmq::socket_t* socket);
    };
}
