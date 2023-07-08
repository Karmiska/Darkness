#include "engine/graphics/vulkan/VulkanSampler.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanDescriptorHandle.h"
#include "engine/graphics/vulkan/VulkanConversions.h"

#include "tools/Debug.h"

int SamplerCount = 0;

namespace engine
{
    namespace implementation
    {
        SamplerImplVulkan::SamplerImplVulkan(
            const Device& device,
            const SamplerDescription& desc)
            : m_device{ device }
            , m_sampler{ vulkanPtr<VkSampler>(static_cast<const DeviceImplVulkan*>(device.native())->device(), vkDestroySampler) }
        {
            VkSamplerCreateInfo samplerInfo = vulkanSamplerState(desc);

			if (desc.desc.filter == Filter::Trilinear)
			{
				LOG("TODO: Cubic filtering is not working");
				samplerInfo.magFilter = VK_FILTER_LINEAR;
				samplerInfo.minFilter = VK_FILTER_LINEAR;
			}

            auto result = vkCreateSampler(static_cast<const DeviceImplVulkan*>(device.native())->device(), &samplerInfo, nullptr, m_sampler.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_sampler.get());
            debInfo.pObjectName = SamplerDebugNames[SamplerCount++];
            debInfo.objectType = VK_OBJECT_TYPE_SAMPLER;
            result = SetDebugUtilsObjectNameEXT(static_cast<const DeviceImplVulkan*>(device.native())->device(), &debInfo);
            ASSERT(result == VK_SUCCESS);
        }

        const VkSampler& SamplerImplVulkan::native() const
        {
            return *m_sampler;
        }

        VkSampler& SamplerImplVulkan::native()
        {
            return *m_sampler;
        }
    }
}
