#pragma once

#include "engine/graphics/Format.h"
#include "containers/vector.h"

namespace engine
{
    enum class ResourceClientMessage
    {
        GetCores,
        CoresAnswer,
        TaskConfirm,
        ShutdownResourceClient,
        TaskHelo,
        ImageEncodingTask,
        ImageEncodingStart,
        ImageEncodingProgress,
        ImageEncodingDone,
        TestHelo,
        TestEhlo,
        TaskResultsReceived
    };

    enum class DiscoveryMessage
    {
        Helo,
        Ehlo
    };

    struct EncodingTask
    {
        uint64_t    taskId;
        uint32_t    sourceWidth;
        uint32_t    sourceHeight;
        size_t      sourceSizeBytes;
        int         sourceFormat;
        uint32_t    sourceStride;
        size_t      destinationSizeBytes;
        int         destinationFormat;
        Format      encodingFormat;

        engine::vector<char> sourceBuffer;
    };

    engine::vector<char> encodeTask(const EncodingTask& task);
    EncodingTask decodeTask(const engine::vector<char>& data);

    struct EncodingTaskResult
    {
        engine::vector<char> buffer;
        uint64_t taskId;
    };

    engine::vector<char> encodeTaskResult(const EncodingTaskResult& task);
    EncodingTaskResult decodeTaskResult(const engine::vector<char>& data);
}
