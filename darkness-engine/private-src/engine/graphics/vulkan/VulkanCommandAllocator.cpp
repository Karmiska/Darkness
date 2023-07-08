#include "engine/graphics/vulkan/VulkanCommandAllocator.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanConversions.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/vulkan/VulkanDevice.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        CommandAllocatorImplVulkan::CommandAllocatorImplVulkan(const DeviceImplVulkan& device, CommandListType type)
            : m_device{ device.device() }
            , m_commandAllocator{ vulkanPtr<VkCommandPool>(m_device, vkDestroyCommandPool) }
            , m_type{ type }
        {
            engine::vector<VulkanQueue> deviceQueues = device.deviceQueues();
            int graphicsFamily = -1;
            int presentFamily = -1;
            int queueIndex = 0;
            //vulkanCommandListType
            if ((type == CommandListType::Direct) || (type == CommandListType::Compute))
            {
                //for (const auto& queue : deviceQueues)
                for (auto queue = deviceQueues.rbegin(); queue != deviceQueues.rend(); ++queue)
                {
                    if ((*queue).taken.size() > 0 && (*queue).flags & VK_QUEUE_GRAPHICS_BIT)
                        graphicsFamily = static_cast<int>(deviceQueues.size()) - queueIndex - 1;

                    if ((*queue).taken.size() > 0 && (*queue).presentSupport)
                        presentFamily = static_cast<int>(deviceQueues.size()) - queueIndex - 1;

                    if (graphicsFamily >= 0 && presentFamily >= 0)
                        break;

                    ++queueIndex;
                }
            }
            else if (type == CommandListType::Copy)
            {
                //for (const auto& queue : deviceQueues)
                for(auto queue = deviceQueues.rbegin(); queue != deviceQueues.rend(); ++queue)
                {
                    if ((*queue).taken.size() > 0 && (*queue).flags & VK_QUEUE_TRANSFER_BIT)
                        graphicsFamily = static_cast<int>(deviceQueues.size()) - queueIndex - 1;

                    if (graphicsFamily >= 0)
                        break;

                    ++queueIndex;
                }
            }
            else
                ASSERT(false, "Not supported");


            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = static_cast<uint32_t>(graphicsFamily);
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            auto result = vkCreateCommandPool(
                device.device(),
                &poolInfo,
                nullptr,
                m_commandAllocator.get());
            ASSERT(result == VK_SUCCESS);
        }

        VkCommandPool& CommandAllocatorImplVulkan::native()
        {
            return *m_commandAllocator;
        }

        const VkCommandPool& CommandAllocatorImplVulkan::native() const
        {
            return *m_commandAllocator;
        }

        void CommandAllocatorImplVulkan::reset()
        {
            auto result = vkResetCommandPool(m_device, *m_commandAllocator, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
            ASSERT(result == VK_SUCCESS);
        }

        CommandListType CommandAllocatorImplVulkan::type() const
        {
            return m_type;
        }
    }
}
