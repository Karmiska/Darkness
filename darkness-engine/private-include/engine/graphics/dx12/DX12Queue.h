#pragma once

#include "engine/graphics/QueueImplIf.h"
#include "containers/vector.h"
#include "containers/memory.h"

struct ID3D12CommandQueue;

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
        class QueueImplDX12 : public QueueImplIf
        {
        public:
            QueueImplDX12(Device& device, CommandListType type, const char* queueName);
            ~QueueImplDX12();

            QueueImplDX12(const QueueImplDX12&) = delete;
            QueueImplDX12(QueueImplDX12&&) = delete;
            QueueImplDX12& operator=(const QueueImplDX12&) = delete;
            QueueImplDX12& operator=(QueueImplDX12&&) = delete;

            void submit(engine::vector<CommandList>& commandLists);
            void submit(engine::vector<CommandList>& commandLists, Fence& fence);
            void submit(engine::vector<CommandList>& commandLists, Semaphore& semaphore);

            void submit(CommandList& commandList) override;
            void submit(CommandList& commandList, Fence& fence) override;
			void submit(CommandList& commandList, Semaphore& semaphore) override;
			void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore) override;
			void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore, Fence& fence) override;
			void submit(CommandList& commandList, Semaphore& semaphore, Fence& fence) override;

            void waitForIdle() const override;

            void signal(const Semaphore& semaphore) override;
            void signal(const Fence& fence, unsigned long long value) override;

            void present(
                Semaphore& signalSemaphore,
                SwapChain& swapChain,
                unsigned int chainIndex) override;

            bool needRefresh() const override;

            ID3D12CommandQueue* native();
            uint64_t timeStampFrequency() const override
            {
                return m_timeStampFrequency;
            }

            engine::Vector3<size_t> transferGranularity() const { return { 1ull, 1ull, 1ull }; }

        private:
            Device& m_device;
            ID3D12CommandQueue* m_queue;
            const char* m_queueName;
            engine::shared_ptr<Fence> m_waitForClearFence;
            uint64_t m_timeStampFrequency;
        };
    }
}

