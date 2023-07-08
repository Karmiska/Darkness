#pragma once

#include "engine/graphics/QueueImplIf.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"

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
        struct QueueInfo;

        class QueueImplVulkan : public QueueImplIf
        {
        public:
            QueueImplVulkan(const Device& device, CommandListType type, const char* queueName);
            ~QueueImplVulkan();

            QueueImplVulkan(const QueueImplVulkan&) = delete;
            QueueImplVulkan(QueueImplVulkan&&) = delete;
            QueueImplVulkan& operator=(const QueueImplVulkan&) = delete;
            QueueImplVulkan& operator=(QueueImplVulkan&&) = delete;

            void submit(CommandList& commandList) override;
            void submit(CommandList& commandList, Fence& fence) override;
            void submit(CommandList& commandList, Semaphore& semaphore) override;
            void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore) override;
            void submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore, Fence& fence) override;
            void submit(CommandList& commandList, Semaphore& semaphore, Fence& fence) override;

            void submit(engine::vector<CommandList>& commandLists);
            void submit(engine::vector<CommandList>& commandLists, Fence& fence);
            void submit(engine::vector<CommandList>& commandLists, Semaphore& semaphore);

            void waitForIdle() const override;

            void signal(const Semaphore& semaphore) override;
            void signal(const Fence& fence, unsigned long long value) override;

            void present(
                Semaphore& signalSemaphore,
                SwapChain& swapChain,
                unsigned int chainIndex) override;

            bool needRefresh() const override;

            const VkQueue& native() const;

            uint64_t timeStampFrequency() const override
            {
                return 1u;
            }

            engine::Vector3<size_t> transferGranularity() const;
        private:
            VkQueue m_queue;
            const Device& m_device;
            engine::unique_ptr<QueueInfo> m_queueInfo;
            bool m_needRefresh;
        };
    }
}

