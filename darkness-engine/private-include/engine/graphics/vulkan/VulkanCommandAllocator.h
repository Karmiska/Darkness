#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/CommandAllocatorImplIf.h"

namespace engine
{
    namespace implementation
    {
        class DeviceImplVulkan;

        class CommandAllocatorImplVulkan : public CommandAllocatorImplIf
        {
        public:
            CommandAllocatorImplVulkan(const DeviceImplVulkan& device, CommandListType type);

            void reset() override;
            CommandListType type() const override;

            VkCommandPool& native();
            const VkCommandPool& native() const;
            
        private:
            VkDevice m_device;
            engine::shared_ptr<VkCommandPool> m_commandAllocator;
            CommandListType m_type;
        };
    }
}

