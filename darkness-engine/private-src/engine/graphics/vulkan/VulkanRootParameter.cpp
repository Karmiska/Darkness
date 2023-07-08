#include "engine/graphics/vulkan/VulkanRootParameter.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanConversions.h"
#include "engine/graphics/vulkan/VulkanDevice.h"

#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/Device.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        RootParameterImplVulkan::RootParameterImplVulkan()
            : m_layoutBinding{}
        {
            m_layoutBinding.binding = 0;
            m_layoutBinding.descriptorCount = 1;
            m_layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
            m_layoutBinding.pImmutableSamplers = VK_NULL_HANDLE;
            m_layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        }

        void RootParameterImplVulkan::binding(unsigned int index)
        {
            m_layoutBinding.binding = static_cast<uint32_t>(index);
        }

        unsigned int RootParameterImplVulkan::binding() const
        {
            return static_cast<unsigned int>(m_layoutBinding.binding);
        }

        void RootParameterImplVulkan::visibility(ShaderVisibility visibility)
        {
            m_layoutBinding.stageFlags = vulkanShaderVisibility(visibility);
        }

        ShaderVisibility RootParameterImplVulkan::visibility() const
        {
            return fromVulkanShaderVisibility(m_layoutBinding.stageFlags);
        }

        void RootParameterImplVulkan::descriptorType(DescriptorType type)
        {
            m_layoutBinding.descriptorType = vulkanDescriptorType(type);
        }

        DescriptorType RootParameterImplVulkan::descriptorType() const
        {
            return fromVulkanDescriptorType(m_layoutBinding.descriptorType);
        }

        const VkDescriptorSetLayoutBinding& RootParameterImplVulkan::layoutBinding() const
        {
            return m_layoutBinding;
        }

        VkDescriptorSetLayoutBinding& RootParameterImplVulkan::layoutBinding()
        {
            return m_layoutBinding;
        }

        void RootParameterImplVulkan::initAsConstants(unsigned int /*reg*/, unsigned int /*num32BitValues*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }

        void RootParameterImplVulkan::initAsCBV(unsigned int /*reg*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }

        void RootParameterImplVulkan::initAsSRV(unsigned int /*reg*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }

        void RootParameterImplVulkan::initAsUAV(unsigned int /*reg*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }

        void RootParameterImplVulkan::initAsDescriptorRange(DescriptorRangeType /*type*/, unsigned int /*reg*/, unsigned int /*count*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }

        void RootParameterImplVulkan::initAsDescriptorTable(unsigned int /*rangeCount*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }

        void RootParameterImplVulkan::setTableRange(unsigned int /*rangeIndex*/, DescriptorRangeType /*type*/, unsigned int /*reg*/, unsigned int /*count*/, unsigned int /*space*/)
        {
            // TODO
        }
    }
}
