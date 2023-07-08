#include "engine/resources/ResourceMessages.h"

namespace engine
{
    engine::vector<char> encodeTask(const EncodingTask& task)
    {
        engine::vector<char> buffer(
            sizeof(ResourceClientMessage) +
            sizeof(uint64_t) +
            sizeof(uint32_t) +
            sizeof(uint32_t) +
            sizeof(size_t) +
            sizeof(int) +
            sizeof(uint32_t) +
            sizeof(size_t) +
            sizeof(int) +
            sizeof(Format) +
            task.sourceSizeBytes
        );

        ResourceClientMessage msg = ResourceClientMessage::ImageEncodingTask;

        uint32_t ptr = 0;
        memcpy(&buffer[ptr], &msg, sizeof(ResourceClientMessage)); ptr += sizeof(ResourceClientMessage);
        memcpy(&buffer[ptr], &task.taskId, sizeof(uint64_t)); ptr += sizeof(uint64_t);
        memcpy(&buffer[ptr], &task.sourceWidth, sizeof(uint32_t)); ptr += sizeof(uint32_t);
        memcpy(&buffer[ptr], &task.sourceHeight, sizeof(uint32_t)); ptr += sizeof(uint32_t);
        memcpy(&buffer[ptr], &task.sourceSizeBytes, sizeof(size_t)); ptr += sizeof(size_t);
        memcpy(&buffer[ptr], &task.sourceFormat, sizeof(int)); ptr += sizeof(int);
        memcpy(&buffer[ptr], &task.sourceStride, sizeof(uint32_t)); ptr += sizeof(uint32_t);
        memcpy(&buffer[ptr], &task.destinationSizeBytes, sizeof(size_t)); ptr += sizeof(size_t);
        memcpy(&buffer[ptr], &task.destinationFormat, sizeof(int)); ptr += sizeof(int);
        memcpy(&buffer[ptr], &task.encodingFormat, sizeof(Format)); ptr += sizeof(Format);
        memcpy(&buffer[ptr], &task.sourceBuffer[0], task.sourceSizeBytes);

        return buffer;
    }

    EncodingTask decodeTask(const engine::vector<char>& data)
    {
        EncodingTask task;
        uint32_t ptr = 0;
        memcpy(&task.taskId, &data[ptr], sizeof(uint64_t)); ptr += sizeof(uint64_t);
        memcpy(&task.sourceWidth, &data[ptr], sizeof(uint32_t)); ptr += sizeof(uint32_t);
        memcpy(&task.sourceHeight, &data[ptr], sizeof(uint32_t)); ptr += sizeof(uint32_t);
        memcpy(&task.sourceSizeBytes, &data[ptr], sizeof(size_t)); ptr += sizeof(size_t);
        memcpy(&task.sourceFormat, &data[ptr], sizeof(int)); ptr += sizeof(int);
        memcpy(&task.sourceStride, &data[ptr], sizeof(uint32_t)); ptr += sizeof(uint32_t);
        memcpy(&task.destinationSizeBytes, &data[ptr], sizeof(size_t)); ptr += sizeof(size_t);
        memcpy(&task.destinationFormat, &data[ptr], sizeof(int)); ptr += sizeof(int);
        memcpy(&task.encodingFormat, &data[ptr], sizeof(Format)); ptr += sizeof(Format);

        task.sourceBuffer.resize(task.sourceSizeBytes);
        memcpy(&task.sourceBuffer[0], &data[ptr], task.sourceSizeBytes);
        return task;
    }

    engine::vector<char> encodeTaskResult(const EncodingTaskResult& task)
    {
        size_t bufferSize = task.buffer.size();
        engine::vector<char> buffer(
            sizeof(ResourceClientMessage) +
            sizeof(uint64_t) +
            sizeof(size_t) +
            task.buffer.size()
        );

        ResourceClientMessage msg = ResourceClientMessage::ImageEncodingDone;

        uint32_t ptr = 0;
        memcpy(&buffer[ptr], &msg, sizeof(ResourceClientMessage)); ptr += sizeof(ResourceClientMessage);
        memcpy(&buffer[ptr], &task.taskId, sizeof(uint64_t)); ptr += sizeof(uint64_t);
        memcpy(&buffer[ptr], &bufferSize, sizeof(size_t)); ptr += sizeof(size_t);
        memcpy(&buffer[ptr], &task.buffer[0], task.buffer.size());

        return buffer;
    }

    EncodingTaskResult decodeTaskResult(const engine::vector<char>& data)
    {
        size_t bufferSize;
        uint64_t taskId;
        uint32_t ptr = 0;
        memcpy(&taskId, &data[ptr], sizeof(uint64_t)); ptr += sizeof(uint64_t);
        memcpy(&bufferSize, &data[ptr], sizeof(size_t)); ptr += sizeof(size_t);
        EncodingTaskResult task;
        task.buffer.resize(bufferSize);
        task.taskId = taskId;
        memcpy(&task.buffer[0], &data[ptr], bufferSize);
        return task;
    }
}
