#include "engine/graphics/vulkan/VulkanSemaphore.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/vulkan/VulkanDevice.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        SemaphoreImplVulkan::SemaphoreImplVulkan(const Device& device)
            : m_device{ device }
            , m_semaphore{ vulkanPtr<VkSemaphore>(static_cast<const DeviceImplVulkan*>(device.native())->device(), vkDestroySemaphore) }
        {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            auto result = vkCreateSemaphore(
                static_cast<const DeviceImplVulkan*>(device.native())->device(),
                &semaphoreInfo,
                nullptr,
                m_semaphore.get());
            ASSERT(result == VK_SUCCESS);
        }

        VkSemaphore& SemaphoreImplVulkan::native()
        {
            return *m_semaphore;
        }

        const VkSemaphore& SemaphoreImplVulkan::native() const
        {
            return *m_semaphore;
        }

        void SemaphoreImplVulkan::reset()
        {
            LOG("TODO: SemaphoreImpl::reset on Vulkan");
        }

        bool SemaphoreImplVulkan::signaled() const
        {
            LOG("TODO: SemaphoreImpl::signaled on Vulkan");
            return false;
        }

        FenceValue SemaphoreImplVulkan::currentGPUValue() const
        {
            FenceValue completedValue;
            auto res = GetSemaphoreCounterValueKHR(static_cast<const DeviceImplVulkan*>(m_device.native())->device(), *m_semaphore, &completedValue);
            ASSERT(res == VK_SUCCESS, "Failed to get initial fence value");
            return completedValue;
        }
    }
}
