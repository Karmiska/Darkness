#include "engine/graphics/vulkan/VulkanRootSignature.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanRootParameter.h"
#include "engine/graphics/vulkan/VulkanSampler.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanConversions.h"

#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/RootSignature.h"
#include "engine/graphics/Device.h"

#include "tools/Debug.h"
#include "tools/ComPtr.h"

namespace engine
{
    namespace implementation
    {
        RootSignatureImplVulkan::RootSignatureImplVulkan(const Device& device, int rootParameterCount, int staticSamplerCount)
            : m_device{ device }
            , m_bindlessDescriptorHeap{ vulkanPtr<VkDescriptorPool>(
                static_cast<const DeviceImplVulkan*>(device.native())->device(), 
                vkDestroyDescriptorPool) }
            , m_numInitializedStaticSamplers{ 0 }
            , m_samplers{ nullptr }
            , m_parameterCount{ 0 }
            , m_samplerCount{ 0 }
            , m_descriptorSetLayout{ vulkanPtr<VkDescriptorSetLayout>(static_cast<const DeviceImplVulkan*>(m_device.native())->device(), vkDestroyDescriptorSetLayout) }
            , m_enableNullDescriptors{ false }
            , m_texture{ false }
            , m_writeable{ false }
        {
            reset(rootParameterCount, staticSamplerCount);

            engine::vector<VkDescriptorPoolSize> poolSizes;
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 15000 });        // Texture SRV
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 15000 });        // Texture UAV
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 15000 }); // Buffer TYPED SRV
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 15000 });       // Buffer STRUCTURED

            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 10000;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            auto result = vkCreateDescriptorPool(
                static_cast<const DeviceImplVulkan*>(device.native())->device(),
                &poolInfo,
                nullptr,
                m_bindlessDescriptorHeap.get());
            ASSERT(result == VK_SUCCESS);

        }

        void RootSignatureImplVulkan::reset(int rootParameterCount, int staticSamplerCount)
        {
            m_parameterCount = rootParameterCount;
            m_samplerCount = staticSamplerCount;

            m_parameters.clear();
            for (int i = 0; i < rootParameterCount; ++i)
                m_parameters.emplace_back(RootParameter(GraphicsApi::Vulkan));

            if (staticSamplerCount > 0)
                m_samplers.reset(new StaticSamplerDescription[static_cast<size_t>(staticSamplerCount)]);
            else
                m_samplers = nullptr;
        }

        VkDescriptorSetLayout& RootSignatureImplVulkan::layout()
        {
            return *m_descriptorSetLayout;
        }

        const VkDescriptorSetLayout& RootSignatureImplVulkan::layout() const
        {
            return *m_descriptorSetLayout;
        }


        void RootSignatureImplVulkan::initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility)
        {
            StaticSamplerDescription& staticSamplerDesc = m_samplers[m_numInitializedStaticSamplers++];
            staticSamplerDesc.desc.filter = description.desc.filter;
            staticSamplerDesc.desc.addressU = description.desc.addressU;
            staticSamplerDesc.desc.addressV = description.desc.addressV;
            staticSamplerDesc.desc.addressW = description.desc.addressW;
            staticSamplerDesc.desc.mipLODBias = description.desc.mipLODBias;
            staticSamplerDesc.desc.maxAnisotrophy = description.desc.maxAnisotrophy;
            staticSamplerDesc.desc.comparisonFunc = description.desc.comparisonFunc;
            staticSamplerDesc.desc.borderColor = StaticBorderColor::OpaqueWhite;
            staticSamplerDesc.desc.minLOD = description.desc.minLOD;
            staticSamplerDesc.desc.maxLOD = description.desc.maxLOD;
            staticSamplerDesc.desc.shaderRegister = static_cast<uint32_t>(samplerNum);
            staticSamplerDesc.desc.registerSpace = 0;
            staticSamplerDesc.desc.shaderVisibility = visibility;

            if (staticSamplerDesc.desc.addressU == TextureAddressMode::Border ||
                staticSamplerDesc.desc.addressV == TextureAddressMode::Border ||
                staticSamplerDesc.desc.addressW == TextureAddressMode::Border)
            {
                if (description.desc.borderColor[3] == 1.0f)
                {
                    if (description.desc.borderColor[0] == 1.0f)
                        staticSamplerDesc.desc.borderColor = StaticBorderColor::OpaqueWhite;
                    else
                        staticSamplerDesc.desc.borderColor = StaticBorderColor::OpaqueBlack;
                }
                else
                    staticSamplerDesc.desc.borderColor = StaticBorderColor::TransparentBlack;
            }
        }

        void RootSignatureImplVulkan::finalize(RootSignatureFlags /*flags*/)
        {
            ASSERT(static_cast<int>(m_numInitializedStaticSamplers) == m_samplerCount);

            

            if (!m_enableNullDescriptors)
            {
                engine::vector<VkDescriptorSetLayoutBinding> bindings;
                for (auto& parameter : m_parameters)
                {
                    bindings.emplace_back(static_cast<RootParameterImplVulkan*>(parameter.native())->layoutBinding());
                }

                VkDescriptorSetLayoutCreateInfo layoutInfo = {};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();

                auto result = vkCreateDescriptorSetLayout(static_cast<const DeviceImplVulkan*>(m_device.native())->device(), &layoutInfo, nullptr, m_descriptorSetLayout.get());
                ASSERT(result == VK_SUCCESS);
            }
            else
            {
                engine::vector<VkDescriptorSetLayoutBinding> bindings;
                bindings.emplace_back(VkDescriptorSetLayoutBinding{});
                bindings[0].binding = 0;
                bindings[0].descriptorCount = 16384;
                bindings[0].descriptorType = m_texture ? 
                    m_writeable ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE :        // Texture UAV
                                  VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE :        // Texture SRV
                    m_writeable ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER :       // Buffer UAV
                                  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;  // Buffer SRV
                bindings[0].pImmutableSamplers = VK_NULL_HANDLE;
                bindings[0].stageFlags = vulkanShaderVisibility(m_visibility);


                VkDescriptorSetLayoutCreateInfo layoutInfo = {};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();



                VkDescriptorSetLayoutBindingFlagsCreateInfo createInfo = {};

                engine::vector<VkDescriptorBindingFlags> flags;
                flags.emplace_back(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);

                createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                createInfo.pBindingFlags = flags.data();
                createInfo.bindingCount = static_cast<uint32_t>(flags.size());
                layoutInfo.pNext = &createInfo;
                /*m_device.createBuffer

                VkDescriptorSetAllocateInfo allocInfo;
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.



                uint32_t counts[1];
                counts[0] = 1000000; // Set 0 has a variable count descriptor with a maximum of 32 elements

                VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts = {};
                set_counts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                set_counts.descriptorSetCount = 1;
                set_counts.pDescriptorCounts = counts;

                createInfo.pNext = &set_counts;*/

                auto result = vkCreateDescriptorSetLayout(static_cast<const DeviceImplVulkan*>(m_device.native())->device(), &layoutInfo, nullptr, m_descriptorSetLayout.get());
                ASSERT(result == VK_SUCCESS);
            }
        }

        void RootSignatureImplVulkan::enableNullDescriptors(bool texture, bool writeable)
        {
            m_enableNullDescriptors = true;
            m_texture = texture;
            m_writeable = writeable;
        }

        size_t RootSignatureImplVulkan::rootParameterCount() const
        {
            return static_cast<size_t>(m_parameterCount);
        }

        RootParameter& RootSignatureImplVulkan::operator[](size_t index)
        {
            ASSERT(index >= 0 && index < m_parameterCount);
            return m_parameters[static_cast<size_t>(index)];
        }

        const RootParameter& RootSignatureImplVulkan::operator[](size_t index) const
        {
            ASSERT(index >= 0 && index < m_parameterCount);
            return m_parameters[static_cast<size_t>(index)];
        }

    }
}
