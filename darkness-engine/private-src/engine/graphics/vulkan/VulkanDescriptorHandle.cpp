#include "engine/graphics/vulkan/VulkanDescriptorHandle.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanDevice.h"

#include "engine/graphics/Device.h"
#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        DescriptorHandleImplVulkan::DescriptorHandleImplVulkan(VkDescriptorSet descriptor)
            : m_descriptor{ descriptor }
        {
        }
        
        DescriptorHandleType DescriptorHandleImplVulkan::type() const
        {
            return m_type;
        }

        DescriptorHandleImplVulkan& DescriptorHandleImplVulkan::operator++()
        {
            return *this;
        }

        DescriptorHandleImplVulkan& DescriptorHandleImplVulkan::operator++(int)
        {
            return *this;
        }

        DescriptorHandleImplVulkan& DescriptorHandleImplVulkan::operator+=(int /*count*/)
        {
            return *this;
        }

        VkDescriptorSet& DescriptorHandleImplVulkan::native()
        {
            return m_descriptor;
        }

        const VkDescriptorSet& DescriptorHandleImplVulkan::native() const
        {
            return m_descriptor;
        }
    }
}
