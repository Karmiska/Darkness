#include "engine/graphics/vulkan/VulkanRenderPass.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanConversions.h"
#include "engine/graphics/Device.h"

namespace engine
{
    namespace implementation
    {
        RenderPass::RenderPass(Device& device, shaders::PipelineConfiguration* configuration)
            : m_device{ device }
            , m_configuration{ configuration }
            , m_subPass{}
            , m_dependency{}

            , m_rtvs{}
            , m_dsvs{}

            , m_attachments{}
            , m_colorAttachmentRef{}
            , m_depthAttachmentRef{}

            , m_renderPassCreated{ false }
            , m_renderPassInfo{}
            , m_renderPass{ vulkanPtr<VkRenderPass>(static_cast<DeviceImplVulkan*>(device.native())->device(), vkDestroyRenderPass) }
        {}

        void RenderPass::createRenderPass()
        {
            m_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            m_dependency.dstSubpass = 0;
            m_dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            m_dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            m_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            m_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            auto attachmentIndex = 0;
            for (auto& rtv : m_rtvs)
            {
                VkAttachmentDescription colorAttachement = {};
                VkAttachmentReference colorAttachmentRef = {};

                colorAttachement.format = vulkanFormat(rtv.format());
                colorAttachement.samples = vulkanSamples(rtv.texture().samples());
                colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachement.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachement.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                colorAttachmentRef.attachment = attachmentIndex;
                colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                m_attachments.emplace_back(std::move(colorAttachement));
                m_colorAttachmentRef.emplace_back(std::move(colorAttachmentRef));

                ++attachmentIndex;
            }
            /*if (m_rtvs.size() == 0 && m_configuration->hasPixelShader())
            {
                VkAttachmentDescription colorAttachement = {};
                VkAttachmentReference colorAttachmentRef = {};

                colorAttachement.format = vulkanFormat(Format::R32_UINT);
                colorAttachement.samples = vulkanSamples(1);
                colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachement.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachement.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                colorAttachmentRef.attachment = VK_ATTACHMENT_UNUSED;
                colorAttachmentRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;

                m_attachments.emplace_back(std::move(colorAttachement));
                m_colorAttachmentRef.emplace_back(std::move(colorAttachmentRef));

                ++attachmentIndex;
            }*/

            for (auto& dsv : m_dsvs)
            {
                VkAttachmentDescription depthAttachment = {};
                VkAttachmentReference depthAttachmentRef = {};

                depthAttachment.format = vulkanFormat(dsv.format());
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                depthAttachmentRef.attachment = attachmentIndex;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                m_attachments.emplace_back(std::move(depthAttachment));
                m_depthAttachmentRef.emplace_back(std::move(depthAttachmentRef));

                ++attachmentIndex;
            }

            m_subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            m_subPass.colorAttachmentCount = static_cast<uint32_t>(m_colorAttachmentRef.size());
            if (m_colorAttachmentRef.size() > 0)
                m_subPass.pColorAttachments = &m_colorAttachmentRef[0];
            else
                m_subPass.pColorAttachments = VK_NULL_HANDLE;
            if(m_depthAttachmentRef.size() > 0)
                m_subPass.pDepthStencilAttachment = &m_depthAttachmentRef[0];
            else
                m_subPass.pDepthStencilAttachment = VK_NULL_HANDLE;

            m_renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            m_renderPassInfo.attachmentCount = static_cast<uint32_t>(m_attachments.size());
            m_renderPassInfo.pAttachments = m_attachments.data();
            m_renderPassInfo.subpassCount = 1;
            m_renderPassInfo.pSubpasses = &m_subPass;

            auto result = vkCreateRenderPass(static_cast<DeviceImplVulkan*>(m_device.native())->device(),
                &m_renderPassInfo, VK_NULL_HANDLE, m_renderPass.get());
            ASSERT(result == VK_SUCCESS);
        }

        void RenderPass::updateAttachments(
            engine::vector<TextureRTV> rtvs,
            engine::vector<TextureDSV> dsvs)
        {
            m_rtvs = rtvs;
            m_dsvs = dsvs;
        }

        VkRenderPass& RenderPass::renderPass()
        {
            if (!m_renderPassCreated)
            {
                createRenderPass();
                m_renderPassCreated = true;
            }
            return *m_renderPass;
        }
    }
}
