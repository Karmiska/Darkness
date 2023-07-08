#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/Device.h"
#include "containers/vector.h"

namespace engine
{
    namespace shaders
    {
        class PipelineConfiguration;
    }

    namespace implementation
    {
        class RenderPass
        {
        public:
            RenderPass(Device& device, shaders::PipelineConfiguration* configuration);

            void updateAttachments(
                engine::vector<TextureRTV> rtvs,
                engine::vector<TextureDSV> dsvs);

            VkRenderPass& renderPass();

			engine::vector<TextureRTV>& rtvs()
			{
				return m_rtvs;
			}

			engine::vector<TextureDSV>& dsvs()
			{
				return m_dsvs;
			}

        private:
            Device& m_device;
            shaders::PipelineConfiguration* m_configuration;
            VkSubpassDescription m_subPass;
            VkSubpassDependency m_dependency;

            engine::vector<TextureRTV> m_rtvs;
            engine::vector<TextureDSV> m_dsvs;

            engine::vector<VkAttachmentDescription> m_attachments;
            engine::vector<VkAttachmentReference> m_colorAttachmentRef;
            engine::vector<VkAttachmentReference> m_depthAttachmentRef;

            bool m_renderPassCreated;
            void createRenderPass();
            VkRenderPassCreateInfo m_renderPassInfo;
            engine::shared_ptr<VkRenderPass> m_renderPass;
        };
    }
}