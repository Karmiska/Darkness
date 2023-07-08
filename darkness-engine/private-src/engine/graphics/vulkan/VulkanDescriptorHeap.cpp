#include "engine/graphics/vulkan/VulkanDescriptorHeap.h"
#include "engine/graphics/vulkan/VulkanDescriptorHandle.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanRootSignature.h"
#include "engine/graphics/vulkan/VulkanRootParameter.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanConversions.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanSampler.h"
#include "engine/graphics/vulkan/VulkanPipeline.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/RootSignature.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Sampler.h"
#include "tools/Debug.h"

#include <array>

namespace engine
{
    namespace implementation
    {
        DescriptorHeapImplVulkan::DescriptorHeapImplVulkan(const DeviceImplVulkan& device)
            : m_device{ device }
            , m_heap{ vulkanPtr<VkDescriptorPool>(device.device(), vkDestroyDescriptorPool) }
        {
            engine::vector<VkDescriptorPoolSize> poolSizes;
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 2048 });               // sampler
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 150000 });        // Texture SRV
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 150000 });        // Texture UAV
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 150000 }); // Buffer TYPED SRV
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 150000 }); // Buffer TYPED UAV
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 150000 });       // cbuffer / ConstantBuffer
            poolSizes.emplace_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 150000 });       // Buffer STRUCTURED

            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 100000;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            auto result = vkCreateDescriptorPool(
                device.device(), 
                &poolInfo, 
                nullptr, 
                m_heap.get());
            ASSERT(result == VK_SUCCESS);
        }
        
        // this is so wrong
        engine::shared_ptr<DescriptorHandleImplVulkan> DescriptorHeapImplVulkan::allocate(
            const RootSignature& signature,
            const shaders::PipelineConfiguration& pipelineConfig
            /*, 
            const Buffer& buffer, 
            const TextureSRV& view,
            const Sampler& sampler,
            int count*/)
        {
            //ASSERT(count <= m_handlesAvailable);
            size_t vsTexSrvCount = 0;
            size_t vsTexUavCount = 0;
            size_t vsBufSrvCount = 0;
            size_t vsBufUavCount = 0;
            size_t vsSamplerCount = 0;

            size_t psTexSrvCount = 0;
            size_t psTexUavCount = 0;
            size_t psBufSrvCount = 0;
            size_t psBufUavCount = 0;
            size_t psSamplerCount = 0;

            if (pipelineConfig.hasVertexShader())
            {
                vsTexSrvCount = pipelineConfig.vertexShader()->texture_srvs().size();
                vsTexUavCount = pipelineConfig.vertexShader()->texture_uavs().size();
                vsBufSrvCount = pipelineConfig.vertexShader()->buffer_srvs().size();
                vsBufUavCount = pipelineConfig.vertexShader()->buffer_uavs().size();
                vsSamplerCount = pipelineConfig.vertexShader()->samplers().size();
            }
            if (pipelineConfig.hasPixelShader())
            {
                psTexSrvCount = pipelineConfig.pixelShader()->texture_srvs().size();
                psTexUavCount = pipelineConfig.pixelShader()->texture_uavs().size();
                psBufSrvCount = pipelineConfig.pixelShader()->buffer_srvs().size();
                psBufUavCount = pipelineConfig.pixelShader()->buffer_uavs().size();
                psSamplerCount = pipelineConfig.vertexShader()->samplers().size();
            }
            if (pipelineConfig.hasGeometryShader())
            {
            }
            if (pipelineConfig.hasHullShader())
            {
            }
            if (pipelineConfig.hasDomainShader())
            {
            }
            if (pipelineConfig.hasComputeShader())
            {
            }

            //size_t resourceCount =
            //    vsTexSrvCount +
            //    vsTexUavCount +
            //    vsBufSrvCount +
            //    vsBufUavCount +
            //    vsSamplerCount +
            //    psTexSrvCount +
            //    psTexUavCount +
            //    psBufSrvCount +
            //    psBufUavCount +
            //    psSamplerCount;


            engine::vector<VkDescriptorSetLayout> layouts{ static_cast<const RootSignatureImplVulkan*>(signature.native())->layout() };

            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = *m_heap;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
            allocInfo.pSetLayouts = layouts.data();
            
            VkDescriptorSet descriptorSet;
            auto setresult = vkAllocateDescriptorSets(m_device.device(), &allocInfo, &descriptorSet);
            ASSERT(setresult == VK_SUCCESS);

            /*VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = BufferImplGet::impl(buffer)->native();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = TextureSRVImplGet::impl(view)->native();
            imageInfo.sampler = SamplerImplGet::impl(sampler)->native();*/

            //std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
            engine::vector<VkWriteDescriptorSet> descriptorWrites;

            const VkBufferView views[2] = { 
                static_cast<BufferSRVImplVulkan*>(pipelineConfig.vertexShader()->buffer_srvs()[0].m_impl)->native(), 
                static_cast<BufferSRVImplVulkan*>(pipelineConfig.vertexShader()->buffer_srvs()[1].m_impl)->native() };

            VkWriteDescriptorSet desc = {};
            desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            desc.dstSet = descriptorSet;
            desc.dstBinding = 0;
            desc.dstArrayElement = 0;
            desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            desc.descriptorCount = 2;
            desc.pTexelBufferView = views;

            descriptorWrites.emplace_back(desc);

            /*for (auto&& res : pipelineConfig.vertexShader()->texture_srvs())
            {
                VkDescriptorImageInfo info = {};
                info.imageView = TextureSRVImplGet::impl(res)->native();
                info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                info.sampler = okay where do we get this now;

                VkWriteDescriptorSet desc;
                desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                desc.dstSet = descriptorSet;
                desc.dstBinding = 0;
                desc.dstArrayElement = 0;
                desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                desc.descriptorCount = 1;
                desc.pBufferInfo = &bufferInfo;
            }
            for (auto&& res : pipelineConfig.vertexShader()->texture_uavs())
            {
                rootSignature[currentIndex].binding(currentIndex);
                rootSignature[currentIndex].descriptorType(DescriptorType::StorageImage);
                rootSignature[currentIndex].visibility(static_cast<ShaderVisibility>(ShaderVisibilityBits::Vertex));
                ++currentIndex;
            }*/
            //for (auto&& res : pipelineConfig.vertexShader()->buffer_srvs())
            //{
                /*VkDescriptorBufferInfo info = {};
                //info.buffer = BufferSRVImplGet::impl(res)->native();
                //info.buffer
                info.offset = 0;
                info.range = BufferSRVImplGet::impl(res)->description().elements *
                    BufferSRVImplGet::impl(res)->description().elementSize;*/

                /*VkWriteDescriptorSet desc = {};
                desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                desc.dstSet = descriptorSet;
                desc.dstBinding = 0;
                desc.dstArrayElement = 0;
                desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                desc.descriptorCount = 1;
                desc.pTexelBufferView = &BufferSRVImplGet::impl(res)->native();

                descriptorWrites.emplace_back(desc);*/
            //}

            /*descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSet;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSet;
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;*/

            vkUpdateDescriptorSets(m_device.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

            return engine::make_shared<DescriptorHandleImplVulkan>(descriptorSet);
        }

        VkDescriptorPool& DescriptorHeapImplVulkan::native()
        {
            return *m_heap;
        }
    }
}
