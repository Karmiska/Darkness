#pragma once

#include "containers/vector.h"

struct MetalQueue{};

namespace engine
{
    class Device;
    class CommandList;
    class Semaphore;
    class SwapChain;
    class Fence;
    enum class CommandListType;
    
    namespace implementation
    {
        class QueueImpl
        {
        public:
            QueueImpl(const Device& device, CommandListType type);
            ~QueueImpl();
            
            QueueImpl(const QueueImpl&) = delete;
            QueueImpl(QueueImpl&&) = delete;
            QueueImpl& operator=(const QueueImpl&) = delete;
            QueueImpl& operator=(QueueImpl&&) = delete;
            
            void submit(engine::vector<CommandList>& commandLists);
            void submit(const CommandList& commandList);
            
            void submit(engine::vector<CommandList>& commandLists, Fence& fence);
            void submit(const CommandList& commandList, Fence& fence);
            
            void waitForIdle() const;
            
            void signal(const Semaphore& semaphore);
            
            void present(
                         const Semaphore& signalSemaphore,
                         const SwapChain& swapChain,
                         unsigned int chainIndex);
            
            bool needRefresh() const;
            
            const MetalQueue& native() const;
        private:
            MetalQueue m_queue;
            const Device& m_device;
            bool m_needRefresh;
        };
    }
}

