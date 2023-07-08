#pragma once

#include "engine/graphics/QueueImplIf.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include "engine/primitives/Vector3.h"
#include "engine/graphics/ResourceOwners.h"

namespace engine
{
    class Device;
    class CommandList;
    class Fence;
    class Semaphore;
    class SwapChain;
    class Buffer;
    enum class CommandListType;
    enum class GraphicsApi;

    class Queue
    {
    public:
        Queue(Device& device, CommandListType type, GraphicsApi api, const char* queueName);

        //void submit(engine::vector<CommandList>& commandLists);
        void submit(CommandList& commandList);
        //void submit(engine::vector<CommandList>& commandLists, Fence& fence);
        void submit(CommandList& commandList, Fence& fence);
        void submit(CommandList& commandList, Semaphore& semaphore);
        void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore);
        void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore, Fence& fence);
        void submit(CommandList& commandList, Semaphore& semaphore, Fence& fence);

        void waitForIdle() const;

        void signal(const Semaphore& semaphore);
        void signal(const Fence& fence, unsigned long long value);

        void present(
            Semaphore& signalSemaphore,
            SwapChain& swapChain,
            unsigned int chainIndex);

        bool needRefresh() const;
        uint64_t timeStampFrequency() const;

        implementation::QueueImplIf* native() { return m_impl.get(); }

        engine::Vector3<size_t> transferGranularity() const;
    private:
        Device& m_device;
        engine::unique_ptr<implementation::QueueImplIf> m_impl;

    private:
        void handleShaderDebug(CommandList& commandList);
        BufferOwner m_debugOutputCpu;
        BufferOwner m_debugOutputCpuCount;

    };
}
