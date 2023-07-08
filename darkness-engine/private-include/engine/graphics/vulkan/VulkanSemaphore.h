#pragma once

#include "engine/graphics/SemaphoreImplIf.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/Fence.h"

namespace engine
{
    class Device;
    namespace implementation
    {
        class SemaphoreImplVulkan : public SemaphoreImplIf
        {
        public:
            SemaphoreImplVulkan(const Device& device);

            VkSemaphore& native();
            const VkSemaphore& native() const;

            void reset();
            bool signaled() const;

            engine::FenceValue currentGPUValue() const;
        private:
            const Device& m_device;
            engine::shared_ptr<VkSemaphore> m_semaphore;
        };
    }
}

