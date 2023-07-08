#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/BarrierImplIf.h"

namespace engine
{
    class CommandList;
    class Buffer;
    class TextureRTV;
    class Semaphore;
    enum class ResourceBarrierFlags;
    enum class ResourceState;

    namespace implementation
    {
        class BarrierImplVulkan : public BarrierImplIf
        {
        public:
            BarrierImplVulkan(
                const CommandList& commandList, 
                ResourceBarrierFlags flags,
                const TextureRTV& resource,
                ResourceState before,
                ResourceState after,
                unsigned int subResource,
                const Semaphore& waitSemaphore,
                const Semaphore& signalSemaphore);
            ~BarrierImplVulkan();

            BarrierImplVulkan(const BarrierImplVulkan&) = delete;
            BarrierImplVulkan(BarrierImplVulkan&&) = delete;
            BarrierImplVulkan& operator=(const BarrierImplVulkan&) = delete;
            BarrierImplVulkan& operator=(BarrierImplVulkan&&) = delete;

            void update(
                ResourceState before,
                ResourceState after) override;

        private:
            VkSemaphore m_waitSem[1];
            VkSemaphore m_signalSemaphores[1];
            VkPipelineStageFlags m_waitStages[1];

            VkSubmitInfo m_submitInfo;
            const CommandList& m_commandList;
        };
    }
}

