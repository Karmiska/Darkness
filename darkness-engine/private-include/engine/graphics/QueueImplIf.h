#pragma once

#include "engine/primitives/Vector3.h"
#include <cstdint>

namespace engine
{
    class CommandList;
    class Semaphore;
    class Fence;
    class SwapChain;

    namespace implementation
    {
        class QueueImplIf
        {
        public:
            virtual ~QueueImplIf() {};

            virtual void submit(CommandList& commandList) = 0;
            virtual void submit(CommandList& commandList, Fence& fence) = 0;
            virtual void submit(CommandList& commandList, Semaphore& semaphore) = 0;
            virtual void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore) = 0;
            virtual void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore, Fence& fence) = 0;
            virtual void submit(CommandList& commandList, Semaphore& semaphore, Fence& fence) = 0;

            virtual void waitForIdle() const = 0;

            virtual void signal(const Semaphore& semaphore) = 0;
            virtual void signal(const Fence& fence, unsigned long long value) = 0;

            virtual void present(
                Semaphore& signalSemaphore,
                SwapChain& swapChain,
                unsigned int chainIndex) = 0;

            virtual bool needRefresh() const = 0;
            virtual uint64_t timeStampFrequency() const = 0;

            virtual engine::Vector3<size_t> transferGranularity() const = 0;
        };
    }
}
