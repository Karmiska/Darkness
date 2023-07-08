#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "tools/Codegen.h"

namespace engine
{
    enum class DescriptorHandleType;
    class Device;

    namespace implementation
    {
        class DescriptorHandleImplVulkan
        {
        public:
            DEFAULT_CONSTRUCTORS(DescriptorHandleImplVulkan)
            DescriptorHandleImplVulkan(VkDescriptorSet descriptor);
            
            DescriptorHandleType type() const;
            VkDescriptorSet& native();
            const VkDescriptorSet& native() const;

            DescriptorHandleImplVulkan& operator++();
            DescriptorHandleImplVulkan& operator++(int);
            DescriptorHandleImplVulkan& operator+=(int count);
        private:
            VkDescriptorSet m_descriptor;
            DescriptorHandleType m_type;
        };
    }
}
