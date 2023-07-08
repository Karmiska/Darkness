#include "engine/graphics/vulkan/VulkanPipeline.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanShaderBinary.h"
#include "engine/graphics/vulkan/VulkanConversions.h"
#include "engine/graphics/vulkan/VulkanRootSignature.h"
#include "engine/graphics/vulkan/VulkanRootParameter.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanSwapChain.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanCommandList.h"
#include "engine/graphics/vulkan/VulkanDescriptorHandle.h"
#include "engine/graphics/vulkan/VulkanDescriptorHeap.h"
#include "engine/graphics/vulkan/VulkanSampler.h"

#include "engine/graphics/PipelineImplIf.h"

#include "engine/graphics/ShaderBinary.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/RootSignature.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/CommandList.h"
#include "shaders/ShaderTypes.h"
//#include "spirv_cross.hpp"
#include "tools/Debug.h"
#include "shaders/ShaderTypes.h"

#include <array>
#include <cstddef>

namespace engine
{
    #include "shaders/core/shared_types/ClusterExecuteIndirect.hlsli"

    namespace implementation
    {
        PipelineImplVulkan::PipelineCache::PipelineCache(const VkDevice& device)
            : m_pipeline{ vulkanPtr<VkPipeline>(device, vkDestroyPipeline) }
            , m_descriptorSets{}
        {
        }

        PipelineImplVulkan::PipelineImplVulkan(
            Device& device,
            ShaderStorage& storage)
            : m_device{ device }
            , m_storage{ storage }
            , m_configuration{ nullptr }
            , m_finalized{ false }

            , m_vertexShader{ nullptr }
            , m_pixelShader{ nullptr }
            , m_geometryShader{ nullptr }
            , m_hullShader{ nullptr }
            , m_domainShader{ nullptr }
            , m_computeShader{ nullptr }

            , m_vertShaderStageInfo{ nullptr }
            , m_fragShaderStageInfo{ nullptr }
            , m_geometryShaderStageInfo{ nullptr }
            , m_computeShaderStageInfo{ nullptr }
            , m_domainShaderStageInfo{ nullptr }
            , m_hullShaderStageInfo{ nullptr }

            , m_inputAssembly{}
            , m_viewport{}
            , m_scissor{}
            , m_rasterizer{}
            , m_multisampling{}
            , m_colorBlendAttachement{}
            , m_colorBlending{}

            , m_pipelineLayoutInfo{}
            , m_depthStencilCreateInfo{}

            , m_bindings{}
            , m_bindingAttributes{}

            , m_pipelineLayout{ vulkanPtr<VkPipelineLayout>(static_cast<DeviceImplVulkan*>(device.native())->device(), vkDestroyPipelineLayout) }
            , m_hashResourceStorage{}
            , m_currentPipelineCache{ nullptr }
            
            //, m_pipeline{ vulkanPtr<VkPipeline>(DeviceImplGet::impl(device).device(), vkDestroyPipeline) }

            , m_depthBufferView{}

            //, m_framebuffers{}
            , m_currentLayouts{}
            , m_pipelineDescriptions{}
            , m_pipelineNames{}
            //, m_descriptorSets{}

            , m_uniform{ nullptr }
            , m_uniformBuffers{}
            , m_srvs{}
            , m_samplers{}
            , m_rootSignatures{}
            , m_useInputLaytout{ false }
        {
            setDefaults();
        }

        void PipelineImplVulkan::setDefaults()
        {
            setDepthStencilState(DepthStencilDescription());
			setRasterizerState(RasterizerDescription());

            m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            m_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            m_inputAssembly.primitiveRestartEnable = VK_FALSE;
        }

        const engine::vector<VkDescriptorSet>& PipelineImplVulkan::descriptorSet() const
        {
            return m_currentPipelineCache->m_descriptorSets;
        }
        
        void PipelineImplVulkan::setDepthBufferView(engine::shared_ptr<TextureDSV> view)
        {
            m_depthBufferView = view;
        }
        
        void PipelineImplVulkan::setRootSignature()
        {
            m_currentLayouts.clear();
            for (auto&& descriptorSet : m_rootSignatures)
            {
                auto layout = static_cast<RootSignatureImplVulkan*>(descriptorSet->native())->layout();
                if(layout)
                    m_currentLayouts.emplace_back(layout);
            }
            m_pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            m_pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_currentLayouts.size());
            m_pipelineLayoutInfo.pSetLayouts = m_currentLayouts.data();
            m_pipelineLayoutInfo.pushConstantRangeCount = 0;
            m_pipelineLayoutInfo.pPushConstantRanges = nullptr;
        }
        
        VkPipelineColorBlendAttachmentState vulkanRenderTargetBlendDesc(const RenderTargetBlendDescription& desc)
        {
            VkPipelineColorBlendAttachmentState result;
            result.colorWriteMask = static_cast<VkColorComponentFlags>(desc.desc.renderTargetWriteMask);
            result.blendEnable = static_cast<VkBool32>(desc.desc.blendEnable);
            result.srcColorBlendFactor = vulkanBlendFactor(desc.desc.srcBlend);
            result.dstColorBlendFactor = vulkanBlendFactor(desc.desc.dstBlend);
            result.srcAlphaBlendFactor = vulkanBlendFactor(desc.desc.srcBlendAlpha);
            result.dstAlphaBlendFactor = vulkanBlendFactor(desc.desc.dstBlendAlpha);
            result.colorBlendOp = vulkanBlendOp(desc.desc.blendOp);
            result.alphaBlendOp = vulkanBlendOp(desc.desc.blendOpAlpha);
            return result;
        }

		void PipelineImplVulkan::setBlendState(const BlendDescription& desc)
		{
			m_blendDescription = desc;
		}

		void PipelineImplVulkan::resolveRTVDSVBlend()
		{
            for (int i = 0; i < m_RTVFormats.size(); ++i)
            {
                m_colorBlendAttachement[i] = vulkanRenderTargetBlendDesc(m_blendDescription.desc.renderTarget[i]);
            }
			for (size_t i = m_RTVFormats.size(); i < 8; ++i)
			{
				m_colorBlendAttachement[i] = {};
			}
            
            m_colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            m_colorBlending.logicOpEnable = VK_FALSE;
            m_colorBlending.logicOp = VK_LOGIC_OP_COPY;
            m_colorBlending.attachmentCount = static_cast<uint32_t>(m_RTVFormats.size());
            m_colorBlending.pAttachments = &m_colorBlendAttachement[0];
            m_colorBlending.blendConstants[0] = 0.0f;
            m_colorBlending.blendConstants[1] = 0.0f;
            m_colorBlending.blendConstants[2] = 0.0f;
            m_colorBlending.blendConstants[3] = 0.0f;
        }
        
        void PipelineImplVulkan::setRasterizerState(const RasterizerDescription& desc)
        {
            m_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            m_rasterizer.depthClampEnable = desc.desc.depthClipEnable ? static_cast<uint32_t>(VK_FALSE) : static_cast<uint32_t>(VK_TRUE);
            m_rasterizer.rasterizerDiscardEnable = VK_FALSE;
            m_rasterizer.polygonMode = vulkanFillMode(desc.desc.fillMode);
            m_rasterizer.lineWidth = 1.0f;
            m_rasterizer.cullMode = vulkanCullMode(desc.desc.cullMode);
            m_rasterizer.frontFace = desc.desc.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
            m_rasterizer.depthBiasEnable = desc.desc.depthBias > 0 ? static_cast<uint32_t>(VK_TRUE) : static_cast<uint32_t>(VK_FALSE);
            m_rasterizer.depthBiasConstantFactor = 0.0f;
            m_rasterizer.depthBiasClamp = desc.desc.depthBiasClamp;
            m_rasterizer.depthBiasSlopeFactor = desc.desc.slopeScaledDepthBias;

            m_multisampling.sampleShadingEnable = desc.desc.multisampleEnable ? static_cast<uint32_t>(VK_TRUE) : static_cast<uint32_t>(VK_FALSE);
        }
        
        void PipelineImplVulkan::setDepthStencilState(const DepthStencilDescription& desc)
        {
            m_depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            m_depthStencilCreateInfo.depthTestEnable = static_cast<VkBool32>(desc.desc.depthEnable);
            m_depthStencilCreateInfo.depthWriteEnable = static_cast<VkBool32>(desc.desc.depthWriteMask == DepthWriteMask::All);
            m_depthStencilCreateInfo.depthCompareOp = vulkanComparisonFunction(desc.desc.depthFunc);
            m_depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
            m_depthStencilCreateInfo.minDepthBounds = 0.0f; // Optional
            m_depthStencilCreateInfo.maxDepthBounds = 1.0f; // Optional
            m_depthStencilCreateInfo.stencilTestEnable = static_cast<VkBool32>(desc.desc.stencilEnable);
            m_depthStencilCreateInfo.front = vulkanStencilOpState(desc.desc.frontFace); // Optional
            m_depthStencilCreateInfo.front.compareMask = desc.desc.stencilReadMask;
            m_depthStencilCreateInfo.front.writeMask = desc.desc.stencilWriteMask;
            m_depthStencilCreateInfo.back = vulkanStencilOpState(desc.desc.backFace); // Optional
            m_depthStencilCreateInfo.back.compareMask = desc.desc.stencilReadMask;
            m_depthStencilCreateInfo.back.writeMask = desc.desc.stencilWriteMask;
        }
        
        void PipelineImplVulkan::setSampleMask(unsigned int /*mask*/)
        {
            LOG("TODO: Vulkan PipelineImplVulkan::setSampleMask not implemented");
        }

        void PipelineImplVulkan::setPrimitiveTopologyType(PrimitiveTopologyType type, bool /*adjacency*/)
        {
            m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            m_inputAssembly.topology = vulkanPrimitiveTopologyType(type);
            m_inputAssembly.primitiveRestartEnable = VK_FALSE;
        }
        
        void PipelineImplVulkan::setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality)
        {
            setRenderTargetFormats({ RTVFormat }, DSVFormat, msaaCount, msaaQuality);
        }
        
        void PipelineImplVulkan::setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int /*msaaCount*/, unsigned int /*msaaQuality*/)
        {
			m_RTVFormats = RTVFormats;
			m_DSVFormat = DSVFormat;
        }
        
        void PipelineImplVulkan::setInputLayout(unsigned int numElements, const InputElementDescription* inputElementDescs)
        {
            m_bindings.resize(1);
            m_bindingAttributes.resize(numElements);

            for (uint32_t i = 0; i < numElements; ++i)
            {
                if (i == 0)
                {
                    m_bindings[i].binding = i;
                    m_bindings[i].stride = inputElementDescs[i].desc.alignedByteOffset;
                    m_bindings[i].inputRate = vulkanInputClassification(inputElementDescs[i].desc.inputSlotClass); // VK_VERTEX_INPUT_RATE_INSTANCE
                }
                m_bindingAttributes[i].binding = inputElementDescs[i].desc.inputSlot;
                m_bindingAttributes[i].format = vulkanFormat(inputElementDescs[i].desc.format);
                m_bindingAttributes[i].location = i;
                m_bindingAttributes[i].offset = inputElementDescs[i].desc.offset;
            }
        }
        
        void PipelineImplVulkan::setPrimitiveRestart(IndexBufferStripCutValue /*value*/)
        {
            // TODO
        }
        
        void PipelineImplVulkan::setShaderStages(shaders::PipelineConfiguration* configuration)
        {
            if (configuration->hasVertexShader())
                setVertexShader(*m_vertexShader);
            if (configuration->hasPixelShader())
                setPixelShader(*m_pixelShader);
            if (configuration->hasGeometryShader())
                setGeometryShader(*m_geometryShader);
            if (configuration->hasHullShader())
                setHullShader(*m_hullShader);
            if (configuration->hasDomainShader())
                setDomainShader(*m_domainShader);
            if (configuration->hasComputeShader())
                setComputeShader(*m_computeShader);
        }

        void PipelineImplVulkan::setVertexShader(const ShaderBinary& shaderBinary)
        {
            m_vertShaderStageInfo = engine::make_unique<VkPipelineShaderStageCreateInfo>();
            m_vertShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_vertShaderStageInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
            m_vertShaderStageInfo->module = static_cast<const ShaderBinaryImplVulkan*>(shaderBinary.native())->native();
            m_vertShaderStageInfo->pName = "main";
            m_vertShaderStageInfo->pSpecializationInfo = nullptr; // this can be used to add defines and such
        }
        
        void PipelineImplVulkan::setPixelShader(const ShaderBinary& shaderBinary)
        {
            m_fragShaderStageInfo = engine::make_unique<VkPipelineShaderStageCreateInfo>();
            m_fragShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_fragShaderStageInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            m_fragShaderStageInfo->module = static_cast<const ShaderBinaryImplVulkan*>(shaderBinary.native())->native();
            m_fragShaderStageInfo->pName = "main";
            m_fragShaderStageInfo->pSpecializationInfo = nullptr; // this can be used to add defines and such
        }
        
        void PipelineImplVulkan::setGeometryShader(const ShaderBinary& shaderBinary)
        {
            m_geometryShaderStageInfo = engine::make_unique<VkPipelineShaderStageCreateInfo>();
            m_geometryShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_geometryShaderStageInfo->stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            m_geometryShaderStageInfo->module = static_cast<const ShaderBinaryImplVulkan*>(shaderBinary.native())->native();
            m_geometryShaderStageInfo->pName = "main";
            m_geometryShaderStageInfo->pSpecializationInfo = nullptr;
        }
        
        void PipelineImplVulkan::setHullShader(const ShaderBinary& shaderBinary)
        {
            m_hullShaderStageInfo = engine::make_unique<VkPipelineShaderStageCreateInfo>();
            m_hullShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_hullShaderStageInfo->stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            m_hullShaderStageInfo->module = static_cast<const ShaderBinaryImplVulkan*>(shaderBinary.native())->native();
            m_hullShaderStageInfo->pName = "main";
            m_hullShaderStageInfo->pSpecializationInfo = nullptr;
        }
        
        void PipelineImplVulkan::setDomainShader(const ShaderBinary& shaderBinary)
        {
            m_domainShaderStageInfo = engine::make_unique<VkPipelineShaderStageCreateInfo>();
            m_domainShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_domainShaderStageInfo->stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            m_domainShaderStageInfo->module = static_cast<const ShaderBinaryImplVulkan*>(shaderBinary.native())->native();
            m_domainShaderStageInfo->pName = "main";
            m_domainShaderStageInfo->pSpecializationInfo = nullptr;
        }

        void PipelineImplVulkan::setComputeShader(const ShaderBinary& shaderBinary)
        {
            m_computeShaderStageInfo = engine::make_unique<VkPipelineShaderStageCreateInfo>();
            m_computeShaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            m_computeShaderStageInfo->stage = VK_SHADER_STAGE_COMPUTE_BIT;
            m_computeShaderStageInfo->module = static_cast<const ShaderBinaryImplVulkan*>(shaderBinary.native())->native();
            m_computeShaderStageInfo->pName = "main";
            m_computeShaderStageInfo->pSpecializationInfo = nullptr;
        }
        void PipelineImplVulkan::configure(CommandListImplIf* /*commandList*/, shaders::PipelineConfiguration* /*configuration*/)
        {
            //m_colorAttachement.format = SwapChainImplGet::impl(*swapChain).surfaceFormat().format;
        }
        
        void PipelineImplVulkan::loadShaders(shaders::PipelineConfiguration* configuration)
        {
            if (configuration->hasVertexShader())
            {
                m_vertexShader = configuration->vertexShader()->load(m_device, m_storage, GraphicsApi::Vulkan);
                m_vertexShader->registerForChange(this, [this]() { this->onShaderChange(); });
            }
            else m_vertexShader = nullptr;

            if (configuration->hasPixelShader())
            {
                m_pixelShader = configuration->pixelShader()->load(m_device, m_storage, GraphicsApi::Vulkan);
                m_pixelShader->registerForChange(this, [this]() { this->onShaderChange(); });
            }
            else m_pixelShader = nullptr;

            if (configuration->hasGeometryShader())
            {
                m_geometryShader = configuration->geometryShader()->load(m_device, m_storage, GraphicsApi::Vulkan);
                m_geometryShader->registerForChange(this, [this]() { this->onShaderChange(); });
            }
            else m_geometryShader = nullptr;

            if (configuration->hasHullShader())
            {
                m_hullShader = configuration->hullShader()->load(m_device, m_storage, GraphicsApi::Vulkan);
                m_hullShader->registerForChange(this, [this]() { this->onShaderChange(); });
            }
            else m_hullShader = nullptr;

            if (configuration->hasDomainShader())
            {
                m_domainShader = configuration->domainShader()->load(m_device, m_storage, GraphicsApi::Vulkan);
                m_domainShader->registerForChange(this, [this]() { this->onShaderChange(); });
            }
            else m_domainShader = nullptr;

            if (configuration->hasComputeShader())
            {
                m_computeShader = configuration->computeShader()->load(m_device, m_storage, GraphicsApi::Vulkan);
                m_computeShader->registerForChange(this, [this]() { this->onShaderChange(); });
            }
            else m_computeShader = nullptr;
        }

        void PipelineImplVulkan::onShaderChange()
        {
            m_finalized = false;
            m_hashResourceStorage.clear();

#if 0
            auto pipelineHash = configuration->hash();
            //pipelineHash = m_pipelineState.hash(pipelineHash, configuration->computeShader() != nullptr);

            auto storedPipeline = m_hashResourceStorage.find(pipelineHash);  





            loadShaders(m_configuration);
            setShaderStages(m_configuration);
            createRootSignature();

            auto result = vkCreatePipelineLayout(
                static_cast<DeviceImplVulkan*>(m_device.native())->device(),
                &m_pipelineLayoutInfo,
                nullptr,
                m_pipelineLayout.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_pipelineLayout.get());
            debInfo.pObjectName = m_configuration->pipelineName();
            debInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
            auto res = SetDebugUtilsObjectNameEXT(
                static_cast<const DeviceImplVulkan*>(m_device.native())->device(), &debInfo);
            ASSERT(res == VK_SUCCESS);
#endif
        }

        RenderPass& PipelineImplVulkan::renderPass(shaders::PipelineConfiguration* configuration)
        {
            ASSERT(!(!configuration && !m_renderPass), "Render pass needs configuration to create the correct attachments");
            if(configuration && !m_renderPass)
                m_renderPass = engine::make_shared<RenderPass>(m_device, configuration);
            return *m_renderPass;
        }

        void PipelineImplVulkan::finalize(CommandListImplVulkan& cmd, shaders::PipelineConfiguration* configuration)
        {
#if 0
            for documentation

            /*auto swapChains = m_device.currentSwapChain().lock();
            ASSERT(swapChains);

            m_viewport = {
                0.0f, 0.0f,
                static_cast<float>(SwapChainImplGet::impl(*swapChains).extent().width),
                static_cast<float>(SwapChainImplGet::impl(*swapChains).extent().height),
                0.0f, 1.0f };

            m_scissor = { { 0, 0 }, SwapChainImplGet::impl(*swapChains).extent() };*/

            /*InputElementDescription vertDesc;
            vertDesc.alignedByteOffset(sizeof(Vertex))
                .format(Format::R32G32B32_FLOAT)
                .inputSlot(0)
                .inputSlotClass(InputClassification::PerVertexData)
                .offset(offsetof(Vertex, pos));

            InputElementDescription colorDesc;
            colorDesc.alignedByteOffset(sizeof(Vertex))
                .format(Format::R32G32B32_FLOAT)
                .inputSlot(0)
                .inputSlotClass(InputClassification::PerVertexData)
                .offset(offsetof(Vertex, color));

            InputElementDescription uvDesc;
            uvDesc.alignedByteOffset(sizeof(Vertex))
                .format(Format::R32G32_FLOAT)
                .inputSlot(0)
                .inputSlotClass(InputClassification::PerVertexData)
                .offset(offsetof(Vertex, texCoord));

            m_pipelineDescriptions.emplace_back(vertDesc);
            m_pipelineDescriptions.emplace_back(colorDesc);
            m_pipelineDescriptions.emplace_back(uvDesc);

            setInputLayout(static_cast<unsigned int>(
                m_pipelineDescriptions.size()),
                m_pipelineDescriptions.data());*/
#endif
            m_configuration = configuration;
            if (!m_finalized)
            {
                m_finalized = true;

                // create pipeline layout

                loadShaders(configuration);
                setShaderStages(configuration);
                createRootSignature();

                auto result = vkCreatePipelineLayout(
                    static_cast<DeviceImplVulkan*>(m_device.native())->device(),
                    &m_pipelineLayoutInfo,
                    nullptr,
                    m_pipelineLayout.get());
                ASSERT(result == VK_SUCCESS);

                VkDebugUtilsObjectNameInfoEXT debInfo = {};
                debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_pipelineLayout.get());
                debInfo.pObjectName = configuration->pipelineName();
                debInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
                auto res = SetDebugUtilsObjectNameEXT(
                    static_cast<const DeviceImplVulkan*>(m_device.native())->device(), &debInfo);
                ASSERT(res == VK_SUCCESS);
            }

            auto pipelineHash = configuration->hash();
            //pipelineHash = m_pipelineState.hash(pipelineHash, configuration->computeShader() != nullptr);
            
            auto storedPipeline = m_hashResourceStorage.find(pipelineHash);
            if (storedPipeline != m_hashResourceStorage.end())
            {
                //LOG("Using cached pipeline: %s", m_configuration->pipelineName());
                m_currentPipelineCache = &storedPipeline->second;
                for(auto&& cup : m_currentPipelineCache->constantUploads)
                    static_cast<DeviceImplVulkan*>(m_device.native())->uploadBuffer(cmd, cup.buffer, cup.range, 0);
            }
            else
            {
                //LOG("Using new pipeline: %s", m_configuration->pipelineName());
                PipelineCache newPipeline(static_cast<DeviceImplVulkan*>(m_device.native())->device());
                auto temp = m_hashResourceStorage.insert(std::pair<uint64_t, PipelineCache>{ pipelineHash, std::move(newPipeline) });
                storedPipeline = m_hashResourceStorage.find(pipelineHash);
                m_currentPipelineCache = &storedPipeline->second;

                createBindings(cmd, configuration);
                createGraphicsPipeline();
            }
        }

        int PipelineImplVulkan::countPipelineSets(shaders::PipelineConfiguration* configuration)
        {
            int res = 0;
            if (configuration->hasVertexShader())
            {
                res += configuration->vertexShader()->setCount();
            }
            if (configuration->hasGeometryShader())
            {
                res += configuration->geometryShader()->setCount();
            }
            if (configuration->hasHullShader())
            {
                res += configuration->hullShader()->setCount();
            }
            if (configuration->hasDomainShader())
            {
                res += configuration->domainShader()->setCount();
            }
            if (configuration->hasPixelShader())
            {
                res += configuration->pixelShader()->setCount();
            }
            if (configuration->hasComputeShader())
            {
                res += configuration->computeShader()->setCount();
            }
            return res;
        }

        void PipelineImplVulkan::createBindings(
            CommandListImplVulkan& cmd, 
            shaders::PipelineConfiguration* configuration)
        {
            engine::vector<VkWriteDescriptorSet> descriptorWrites;

            m_resourceViews = {};
            auto& resourceViews = m_resourceViews;

			auto preallocateResources = [](ResourceViews& views, const shaders::Shader* shader)
			{
                views.samplers.resize(views.samplers.size() + shader->samplers().size());
                views.textureSrvs.resize(views.textureSrvs.size() + shader->texture_srvs().size());
                views.textureUavs.resize(views.textureUavs.size() + shader->texture_uavs().size());

                for (int i = 0; i < shader->buffer_srvs().size(); ++i)
				{
					if (shader->buffer_srvs_is_structured()[i])
					{
						views.bufferSrvsStructured.resize(views.bufferSrvsStructured.size() + 1);
					}
				}
				for (int i = 0; i < shader->buffer_srvs().size(); ++i)
				{
					if (!shader->buffer_srvs_is_structured()[i])
					{
						views.bufferSrvsTyped.resize(views.bufferSrvsTyped.size() + 1);
					}
				}
                for (int i = 0; i < shader->buffer_uavs().size(); ++i)
				{
					if (shader->buffer_uavs_is_structured()[i])
					{
						views.bufferUavsStructured.resize(views.bufferUavsStructured.size() + 1);
                        if(shader->buffer_uavs()[i].valid() && shader->buffer_uavs()[i].desc().append)
                            views.bufferCounterUavs.resize(views.bufferCounterUavs.size() + 1);
					}
				}
				for (int i = 0; i < shader->buffer_uavs().size(); ++i)
				{
					if (!shader->buffer_uavs_is_structured()[i])
					{
						views.bufferUavsTyped.resize(views.bufferUavsTyped.size() + 1);
                        if (shader->buffer_uavs()[i].valid() && shader->buffer_uavs()[i].desc().append)
                            views.bufferCounterUavs.resize(views.bufferCounterUavs.size() + 1);
					}
				}

                for (auto&& res : shader->bindless_texture_srvs())
                {
                    views.textureSrvs.resize(views.textureSrvs.size() + res->size());
                }
                for (auto&& res : shader->bindless_texture_uavs())
                {
                    views.textureUavs.resize(views.textureUavs.size() + res->size());
                }

                views.constants.resize(views.constants.size() + const_cast<shaders::Shader*>(shader)->root_constants().size());
                views.constants.resize(views.constants.size() + const_cast<shaders::Shader*>(shader)->constants().size());
			};

			if (configuration->hasVertexShader())
			{
				preallocateResources(resourceViews, configuration->vertexShader());
			}
			if (configuration->hasGeometryShader())
			{
				preallocateResources(resourceViews, configuration->geometryShader());
			}
			if (configuration->hasHullShader())
			{
				preallocateResources(resourceViews, configuration->hullShader());
			}
			if (configuration->hasDomainShader())
			{
				preallocateResources(resourceViews, configuration->domainShader());
			}
			if (configuration->hasPixelShader())
			{
				preallocateResources(resourceViews, configuration->pixelShader());
			}
			if (configuration->hasComputeShader())
			{
				preallocateResources(resourceViews, configuration->computeShader());
			}


            VkDeviceSize offset = 0;
            auto setupWrites = [this, &cmd](
                ResourceViews& resourceViews,
                VkDescriptorSet& descriptorSet,
                engine::vector<VkWriteDescriptorSet>& writes,
                const shaders::Shader* shader,
                VkDeviceSize& offset)
            {
                // Root Constants
                {
                    auto currentConstants = resourceViews.count_constants;
                    for (auto&& res : const_cast<shaders::Shader*>(shader)->root_constants())
                    {
                        if (!res->buffer)
                        {
                            ClusterExecuteIndirectArgs initialData;
                            const_cast<engine::RootConstant*>(res)->range = tools::ByteRange(
                                reinterpret_cast<const uint8_t*>(static_cast<const ClusterExecuteIndirectArgs*>(&initialData)),
                                reinterpret_cast<const uint8_t*>(static_cast<const ClusterExecuteIndirectArgs*>(&initialData)) + sizeof(ClusterExecuteIndirectArgs));

                            auto initial = BufferDescription::InitialData(res->range, 4);

                            const_cast<engine::RootConstant*>(res)->buffer = engine::make_shared<BufferCBVOwner>(this->m_device.createBufferCBV(BufferDescription()
                                .name(res->name().c_str())
                                .usage(ResourceUsage::GpuRead)
                                .elementSize(initial.elementSize)
                                .elements(initial.elements)
                                .structured(true)));
                        }
                        static_cast<DeviceImplVulkan*>(m_device.native())->uploadBuffer(cmd, *res->buffer, res->range, 0);

                        BufferCBV cbv = res->buffer->resource();
                        VkDescriptorBufferInfo bufferInfo = {};
                        bufferInfo.buffer = static_cast<BufferImplVulkan*>(cbv.buffer().m_impl)->native();
                        bufferInfo.offset = 0;
                        bufferInfo.range = res->range.sizeBytes();
                        resourceViews.constants[resourceViews.count_constants] = bufferInfo;
                        ++resourceViews.count_constants;
                    }
                    auto resCount = resourceViews.count_constants - currentConstants;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pBufferInfo = &resourceViews.constants[currentConstants];
                        writes.emplace_back(desc);
                    }
                }
				
                // Samplers
                {
                    auto currentCount = resourceViews.count_samplers;
                    for (auto&& res : shader->samplers())
                    {
                        VkDescriptorImageInfo imageInfo = {};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        imageInfo.imageView = VK_NULL_HANDLE;
                        if (res.valid())
                            imageInfo.sampler = static_cast<const SamplerImplVulkan*>(res.native())->native();
                        else
                            imageInfo.sampler = static_cast<const SamplerImplVulkan*>(m_device.nullResouces().sampler.native())->native();

                        resourceViews.samplers[resourceViews.count_samplers] = imageInfo;
                        ++resourceViews.count_samplers;
                    }
                    auto resCount = resourceViews.count_samplers - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pImageInfo = &resourceViews.samplers[currentCount];
                        writes.emplace_back(desc);
                    }
                }

                // Texture SRVs
                {
                    auto currentCount = resourceViews.count_textureSrvs;
                    for (size_t i = 0; i < shader->texture_srvs().size(); ++i)
                    {
                        auto& res = shader->texture_srvs()[i];
                        VkDescriptorImageInfo imageInfo = {};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        if (res.valid())
                            imageInfo.imageView = static_cast<TextureSRVImplVulkan*>(res.m_impl)->native();
                        else
                        {
                            auto format = shader->texture_srvs_format()[i];
                            auto dimension = ResourceDimension::Unknown;
                            for (auto&& srv : shader->srvBindings())
                                if (srv.type == shaders::BindingType::SRVTexture && srv.index == i)
                                {
                                    dimension = srv.dimension;
                                    break;
                                }
                            ASSERT(dimension != ResourceDimension::Unknown, "Could not determine dimension");
                            //auto dimension = shader->textureDimension(shader->srvNames()[i]);


                            if (m_device.nullResouces().nullTextureSRV.find(dimension) != m_device.nullResouces().nullTextureSRV.end() &&
                                m_device.nullResouces().nullTextureSRV[dimension].find(format) != m_device.nullResouces().nullTextureSRV[dimension].end())
                            {
                                imageInfo.imageView = static_cast<TextureSRVImplVulkan*>(m_device.nullResouces().nullTextureSRV[dimension][format].resource().m_impl)->native();
                            }
                            else
                            {
                                //imageInfo.imageView = VK_NULL_HANDLE;
                                if (m_device.nullResouces().nullTextureSRV.find(dimension) == m_device.nullResouces().nullTextureSRV.end())
                                    m_device.nullResouces().nullTextureSRV[dimension] = engine::unordered_map<engine::Format, TextureSRVOwner>();

                                m_device.nullResouces().nullTextureSRV[dimension][format] = m_device.createTextureSRV(TextureDescription()
                                    .width(1)
                                    .height(1)
                                    .format(format)
                                    .usage(ResourceUsage::GpuReadWrite)
                                    .dimension(dimension)
                                    .name("Null TextureSRV"));
                                imageInfo.imageView = static_cast<TextureSRVImplVulkan*>(m_device.nullResouces().nullTextureSRV[dimension][format].resource().m_impl)->native();

                                cmd.transition(m_device.nullResouces().nullTextureSRV[dimension][format], ResourceState::PixelShaderResource);

                                auto dimensionToStr = [](ResourceDimension dimension)->const char*
                                {
                                    switch (dimension)
                                    {
                                    case ResourceDimension::Unknown: return "Unknown";
                                    case ResourceDimension::Texture1D: return "Texture1D";
                                    case ResourceDimension::Texture2D: return "Texture2D";
                                    case ResourceDimension::Texture3D: return "Texture3D";
                                    case ResourceDimension::Texture1DArray: return "Texture1DArray";
                                    case ResourceDimension::Texture2DArray: return "Texture2DArray";
                                    case ResourceDimension::TextureCube: return "TextureCube";
                                    case ResourceDimension::TextureCubeArray: return "TextureCubeArray";
                                    }
                                    return "";
                                };

                                LOG("Null format created. Format: %s, dimension: %s",
                                    formatToString(format).c_str(),
                                    dimensionToStr(dimension));
                            }
                        }
                        imageInfo.sampler = VK_NULL_HANDLE;
                        resourceViews.textureSrvs[resourceViews.count_textureSrvs] = imageInfo;
                        ++resourceViews.count_textureSrvs;
                    }
                    auto resCount = resourceViews.count_textureSrvs - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pImageInfo = &resourceViews.textureSrvs[currentCount];
                        writes.emplace_back(desc);
                    }
                }
                
                // Texture UAVs
                {
                    auto currentCount = resourceViews.count_textureUavs;
                    for (size_t i = 0; i < shader->texture_uavs().size(); ++i)
                    {
                        auto& res = shader->texture_uavs()[i];
                        VkDescriptorImageInfo imageInfo = {};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                        if (res.valid())
                            imageInfo.imageView = static_cast<TextureUAVImplVulkan*>(res.m_impl)->native();
                        else
                        {
                            auto format = shader->texture_uavs_format()[i];
                            auto dimension = ResourceDimension::Unknown;
                            for (auto&& uav : shader->uavBindings())
                                if (uav.type == shaders::BindingType::UAVTexture && uav.index == i)
                                {
                                    dimension = uav.dimension;
                                    break;
                                }
                            ASSERT(dimension != ResourceDimension::Unknown, "Could not determine dimension");
                            //auto dimension = shader->textureDimension(shader->uavNames()[i]);


                            if (m_device.nullResouces().nullTextureUAV.find(dimension) != m_device.nullResouces().nullTextureUAV.end() &&
                                m_device.nullResouces().nullTextureUAV[dimension].find(format) != m_device.nullResouces().nullTextureUAV[dimension].end())
                            {
                                imageInfo.imageView = static_cast<TextureUAVImplVulkan*>(m_device.nullResouces().nullTextureUAV[dimension][format].resource().m_impl)->native();
                            }
                            else
                            {
                                if (m_device.nullResouces().nullTextureUAV.find(dimension) == m_device.nullResouces().nullTextureUAV.end())
                                    m_device.nullResouces().nullTextureUAV[dimension] = engine::unordered_map<engine::Format, TextureUAVOwner>();

                                m_device.nullResouces().nullTextureUAV[dimension][format] = m_device.createTextureUAV(TextureDescription()
                                    .width(1)
                                    .height(1)
                                    .format(format)
                                    .usage(ResourceUsage::GpuReadWrite)
                                    .dimension(dimension)
                                    .name("Null TextureUAV"));
                                imageInfo.imageView = static_cast<TextureUAVImplVulkan*>(m_device.nullResouces().nullTextureUAV[dimension][format].resource().m_impl)->native();

                                auto dimensionToStr = [](ResourceDimension dimension)->const char*
                                {
                                    switch (dimension)
                                    {
                                    case ResourceDimension::Unknown: return "Unknown";
                                    case ResourceDimension::Texture1D: return "Texture1D";
                                    case ResourceDimension::Texture2D: return "Texture2D";
                                    case ResourceDimension::Texture3D: return "Texture3D";
                                    case ResourceDimension::Texture1DArray: return "Texture1DArray";
                                    case ResourceDimension::Texture2DArray: return "Texture2DArray";
                                    case ResourceDimension::TextureCube: return "TextureCube";
                                    case ResourceDimension::TextureCubeArray: return "TextureCubeArray";
                                    }
                                    return "";
                                };

                                LOG("Null format created. Format: %s, dimension: %s",
                                    formatToString(format).c_str(),
                                    dimensionToStr(dimension));
                            }
                        }
                        imageInfo.sampler = VK_NULL_HANDLE;
                        resourceViews.textureUavs[resourceViews.count_textureUavs] = imageInfo;
                        ++resourceViews.count_textureUavs;
                    }
                    auto resCount = resourceViews.count_textureUavs - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pImageInfo = &resourceViews.textureUavs[currentCount];
                        writes.emplace_back(desc);
                    }
                }
				
                // Buffer SRVs Structured
                {
                    auto currentCount = resourceViews.count_bufferSrvsStructured;
                    for (int i = 0; i < shader->buffer_srvs().size(); ++i)
                    {
                        if (shader->buffer_srvs_is_structured()[i])
                        {
                            VkDescriptorBufferInfo bufferInfo = {};
                            bufferInfo.offset = 0;
                            if (shader->buffer_srvs()[i].valid())
                            {
                                const BufferSRV& res = shader->buffer_srvs()[i];
                                bufferInfo.buffer = static_cast<BufferImplVulkan*>(res.m_impl->buffer().m_impl)->native();
                                bufferInfo.range = res.desc().elements * res.desc().elementSize;
                            }
                            else
                            {
                                auto res = m_device.nullResouces().bufferStructuredSRV.resource();
                                bufferInfo.buffer = static_cast<BufferImplVulkan*>(res.buffer().m_impl)->native();
                                bufferInfo.range = res.desc().elements * res.desc().elementSize;
                            }
                            resourceViews.bufferSrvsStructured[resourceViews.count_bufferSrvsStructured] = bufferInfo;
                            ++resourceViews.count_bufferSrvsStructured;
                        }
                    }
                    auto resCount = resourceViews.count_bufferSrvsStructured - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pBufferInfo = &resourceViews.bufferSrvsStructured[currentCount];
                        writes.emplace_back(desc);
                    }
                }

                // Buffer SRVs Typed
                {
                    auto currentCount = resourceViews.count_bufferSrvsTyped;
                    for (int i = 0; i < shader->buffer_srvs().size(); ++i)
                    {
                        if (!shader->buffer_srvs_is_structured()[i])
                        {
                            if (shader->buffer_srvs()[i].valid())
                            {
                                const BufferSRV& res = shader->buffer_srvs()[i];
                                resourceViews.bufferSrvsTyped[resourceViews.count_bufferSrvsTyped] = static_cast<BufferSRVImplVulkan*>(res.m_impl)->native();
                            }
                            else
                            {
                                auto format = shader->buffer_srvs_format()[i];

                                if (m_device.nullResouces().nullBufferSRV.find(format) != m_device.nullResouces().nullBufferSRV.end())
                                {
                                    resourceViews.bufferSrvsTyped[resourceViews.count_bufferSrvsTyped] = static_cast<BufferSRVImplVulkan*>(m_device.nullResouces().nullBufferSRV[format].resource().m_impl)->native();
                                }
                                else
                                {
                                    m_device.nullResouces().nullBufferSRV[format] = m_device.createBufferSRV(BufferDescription()
                                        .elements(1)
                                        .format(format)
                                        .usage(ResourceUsage::GpuReadWrite)
                                        .name("Null BufferSRV"));
                                    resourceViews.bufferSrvsTyped[resourceViews.count_bufferSrvsTyped] = static_cast<BufferSRVImplVulkan*>(m_device.nullResouces().nullBufferSRV[format].resource().m_impl)->native();

                                    auto dimensionToStr = [](ResourceDimension dimension)->const char*
                                    {
                                        switch (dimension)
                                        {
                                        case ResourceDimension::Unknown: return "Unknown";
                                        case ResourceDimension::Texture1D: return "Texture1D";
                                        case ResourceDimension::Texture2D: return "Texture2D";
                                        case ResourceDimension::Texture3D: return "Texture3D";
                                        case ResourceDimension::Texture1DArray: return "Texture1DArray";
                                        case ResourceDimension::Texture2DArray: return "Texture2DArray";
                                        case ResourceDimension::TextureCube: return "TextureCube";
                                        case ResourceDimension::TextureCubeArray: return "TextureCubeArray";
                                        }
                                        return "";
                                    };

                                    LOG("Null buffer format created. Format: %s",
                                        formatToString(format).c_str());
                                }
                            }
                            ++resourceViews.count_bufferSrvsTyped;
                        }
                    }
                    auto resCount = resourceViews.count_bufferSrvsTyped - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pTexelBufferView = &resourceViews.bufferSrvsTyped[currentCount];
                        writes.emplace_back(desc);
                    }
                }

                // Buffer UAVs Structured
                {
                    auto currentCount = resourceViews.count_bufferUavsStructured;
                    for (int i = 0; i < shader->buffer_uavs().size(); ++i)
                    {
                        if (shader->buffer_uavs_is_structured()[i])
                        {
                            VkDescriptorBufferInfo bufferInfo = {};
                            bufferInfo.offset = 0;
                            if (shader->buffer_uavs()[i].valid())
                            {
                                const BufferUAV& res = shader->buffer_uavs()[i];
                                bufferInfo.buffer = static_cast<BufferImplVulkan*>(res.m_impl->buffer().m_impl)->native();
                                bufferInfo.range = res.desc().elements * res.desc().elementSize;

                                if (shader->buffer_uavs()[i].desc().append)
                                {
                                    VkDescriptorBufferInfo tmpbufferInfo = {};
                                    tmpbufferInfo.offset = 0;
                                    tmpbufferInfo.buffer = *static_cast<BufferUAVImplVulkan*>(res.m_impl)->m_counterBuffer;
                                    tmpbufferInfo.range = sizeof(uint32_t);
                                    resourceViews.bufferCounterUavs[resourceViews.count_bufferCounterUavs] = tmpbufferInfo;
                                    ++resourceViews.count_bufferCounterUavs;
                                }
                            }
                            else
                            {
                                ASSERT(!shader->buffer_uavs()[i].desc().append, "Missing support for append buffer counter for non bound buffer");
                                auto res = m_device.nullResouces().bufferStructuredUAV.resource();
                                bufferInfo.buffer = static_cast<BufferImplVulkan*>(res.buffer().m_impl)->native();
                                bufferInfo.range = res.desc().elements * res.desc().elementSize;
                            }
                            resourceViews.bufferUavsStructured[resourceViews.count_bufferUavsStructured] = bufferInfo;
                            ++resourceViews.count_bufferUavsStructured;
                        }
                    }
                    auto resCount = resourceViews.count_bufferUavsStructured - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pBufferInfo = &resourceViews.bufferUavsStructured[currentCount];
                        writes.emplace_back(desc);
                    }
                }

                // Buffer UAVs Typed
                {
                    auto currentCount = resourceViews.count_bufferUavsTyped;
                    for (int i = 0; i < shader->buffer_uavs().size(); ++i)
                    {
                        if (!shader->buffer_uavs_is_structured()[i])
                        {
                            if (shader->buffer_uavs()[i].valid())
                            {
                                const BufferUAV& res = shader->buffer_uavs()[i];
                                resourceViews.bufferUavsTyped[resourceViews.count_bufferUavsTyped] = static_cast<BufferUAVImplVulkan*>(res.m_impl)->native();

                                if (shader->buffer_uavs()[i].desc().append)
                                {
                                    VkDescriptorBufferInfo bufferInfo = {};
                                    bufferInfo.offset = 0;
                                    bufferInfo.buffer = *static_cast<BufferUAVImplVulkan*>(res.m_impl)->m_counterBuffer;
                                    bufferInfo.range = sizeof(uint32_t);
                                    resourceViews.bufferCounterUavs[resourceViews.count_bufferCounterUavs] = bufferInfo;
                                    ++resourceViews.count_bufferCounterUavs;
                                }
                            }
                            else
                            {
                                auto res = m_device.nullResouces().bufferUAV.resource();
                                resourceViews.bufferUavsTyped[resourceViews.count_bufferUavsTyped] = static_cast<BufferUAVImplVulkan*>(res.m_impl)->native();
                            }
                            ++resourceViews.count_bufferUavsTyped;
                        }
                    }
                    auto resCount = resourceViews.count_bufferUavsTyped - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pTexelBufferView = &resourceViews.bufferUavsTyped[currentCount];
                        writes.emplace_back(desc);
                    }
                }
                
				// Constants
                {
                    auto currentConstants = resourceViews.count_constants;
                    for (auto&& res : const_cast<shaders::Shader*>(shader)->constants())
                    {
                        if (!res.buffer)
                        {
                            auto initial = BufferDescription::InitialData(res.range, 4);

                            res.buffer = engine::make_shared<BufferCBVOwner>(this->m_device.createBufferCBV(BufferDescription()
                                .name(res.name)
                                .usage(ResourceUsage::GpuRead)
                                .elementSize(initial.elementSize)
                                .elements(initial.elements)
                                .structured(true)));
                        }
                        static_cast<DeviceImplVulkan*>(m_device.native())->uploadBuffer(cmd, *res.buffer, res.range, 0);

                        m_currentPipelineCache->constantUploads.emplace_back(ConstantUpdate{ res.buffer->resource(), res.range });

                        BufferCBV cbv = res.buffer->resource();
                        VkDescriptorBufferInfo bufferInfo = {};
                        bufferInfo.buffer = static_cast<BufferImplVulkan*>(cbv.buffer().m_impl)->native();
                        bufferInfo.offset = 0;
                        bufferInfo.range = res.range.sizeBytes();
                        resourceViews.constants[resourceViews.count_constants] = bufferInfo;
                        ++resourceViews.count_constants;
                    }
                    auto resCount = resourceViews.count_constants - currentConstants;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = descriptorSet;
                        desc.dstBinding = static_cast<uint32_t>(offset); offset += resCount;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pBufferInfo = &resourceViews.constants[currentConstants];
                        writes.emplace_back(desc);
                    }
                }
            };

            auto& currentDescriptorSets = m_currentPipelineCache->m_descriptorSets;

            m_layouts.clear();

            auto gatherSets = [&](const shaders::Shader* shader, int setIndex, int setCount)
            {
                RootSignature& rootSignature = *m_rootSignatures[setIndex];

                auto layoutIndex = m_layouts.size();
                m_layouts.emplace_back(static_cast<RootSignatureImplVulkan*>(rootSignature.native())->layout());

                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = static_cast<DeviceImplVulkan*>(m_device.native())->descriptorHeap().native();
                allocInfo.descriptorSetCount = static_cast<uint32_t>(m_layouts.size()- layoutIndex);
                allocInfo.pSetLayouts = &m_layouts[layoutIndex];

                auto res = vkAllocateDescriptorSets(static_cast<DeviceImplVulkan*>(m_device.native())->device(), &allocInfo, &currentDescriptorSets[setIndex]);
                ASSERT(res == VK_SUCCESS);

                offset = 0;
                setupWrites(resourceViews, currentDescriptorSets[setIndex], descriptorWrites, shader, offset);

                // setup append counter writes if there are any
                for (int i = 0; i < resourceViews.bufferCounterUavs.size(); ++i)
                {
                    VkWriteDescriptorSet desc = {};
                    desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    desc.dstSet = currentDescriptorSets[setIndex];
                    desc.dstBinding = static_cast<uint32_t>(offset); offset += 1;
                    desc.dstArrayElement = 0;
                    desc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    desc.descriptorCount = 1u;
                    desc.pBufferInfo = &resourceViews.bufferCounterUavs[i];
                    descriptorWrites.emplace_back(desc);
                }

                // handle bindless sets
                /*for (int i = 1; i < setCount; ++i)
                {
                    RootSignature& rs = *m_rootSignatures[setIndex+i];
                    auto blayoutIndex = m_layouts.size();
                    m_layouts.emplace_back(static_cast<RootSignatureImplVulkan*>(rs.native())->layout());

                    VkDescriptorSetAllocateInfo ballocInfo = {};
                    ballocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    ballocInfo.descriptorPool = static_cast<DeviceImplVulkan*>(m_device.native())->descriptorHeap().native();
                    ballocInfo.descriptorSetCount = static_cast<uint32_t>(m_layouts.size() - blayoutIndex);
                    ballocInfo.pSetLayouts = &m_layouts[blayoutIndex];

                    auto res = vkAllocateDescriptorSets(static_cast<DeviceImplVulkan*>(m_device.native())->device(), &ballocInfo, &currentDescriptorSets[setIndex + i]);
                    ASSERT(res == VK_SUCCESS);

                    offset = 0;
                    setupWrites(resourceViews, currentDescriptorSets[setIndex + i], descriptorWrites, shader, offset, true);
                }*/
                auto allBindlessCount = 
                    shader->bindless_texture_srvs().size() + 
                    shader->bindless_texture_uavs().size() + 
                    shader->bindless_buffer_srvs().size() + 
                    shader->bindless_buffer_uavs().size();

                // setCount == SRV/UAV/CBV set + bindless sets. (it's also possible there are no SRV/UAV/CBV set)
                size_t bindlessSet = setIndex + (setCount - allBindlessCount);

                for(auto&& bsrv : shader->bindless_texture_srvs())
                {
                    RootSignature& rs = *m_rootSignatures[bindlessSet];
                    auto blayoutIndex = m_layouts.size();
                    m_layouts.emplace_back(static_cast<RootSignatureImplVulkan*>(rs.native())->layout());

                    VkDescriptorSetAllocateInfo ballocInfo = {};
                    ballocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    ballocInfo.descriptorPool = static_cast<DeviceImplVulkan*>(m_device.native())->descriptorHeap().native();
                    ballocInfo.descriptorSetCount = static_cast<uint32_t>(m_layouts.size() - blayoutIndex);
                    ballocInfo.pSetLayouts = &m_layouts[blayoutIndex];

                    uint32_t counts[1];
                    counts[0] = static_cast<uint32_t>(bsrv->size());
                    VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts = {};
                    set_counts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    set_counts.descriptorSetCount = 1;
                    set_counts.pDescriptorCounts = counts;
                    ballocInfo.pNext = &set_counts;

                    res = vkAllocateDescriptorSets(static_cast<DeviceImplVulkan*>(m_device.native())->device(), &ballocInfo, &currentDescriptorSets[bindlessSet]);
                    ASSERT(res == VK_SUCCESS);


                    auto currentCount = resourceViews.count_textureSrvs;
                    for (int i = 0; i < bsrv->size(); ++i)
                    {
                        auto tex = bsrv->get(i);
                        VkDescriptorImageInfo imageInfo = {};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        if (tex.valid())
                            imageInfo.imageView = static_cast<TextureSRVImplVulkan*>(tex.m_impl)->native();
                        else
                            imageInfo.imageView = static_cast<TextureSRVImplVulkan*>(m_device.nullResouces().textureSRV.resource().m_impl)->native();
                        imageInfo.sampler = VK_NULL_HANDLE;

                        resourceViews.textureSrvs[resourceViews.count_textureSrvs] = imageInfo;
                        ++resourceViews.count_textureSrvs;
                    }
                    auto resCount = resourceViews.count_textureSrvs - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = currentDescriptorSets[bindlessSet];
                        desc.dstBinding = 0;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pImageInfo = &resourceViews.textureSrvs[currentCount];
                        descriptorWrites.emplace_back(desc);
                    }

                    ++bindlessSet;
                }
                for (auto&& buav : shader->bindless_texture_uavs())
                {
                    RootSignature& rs = *m_rootSignatures[bindlessSet];
                    auto blayoutIndex = m_layouts.size();
                    m_layouts.emplace_back(static_cast<RootSignatureImplVulkan*>(rs.native())->layout());

                    VkDescriptorSetAllocateInfo ballocInfo = {};
                    ballocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    ballocInfo.descriptorPool = static_cast<DeviceImplVulkan*>(m_device.native())->descriptorHeap().native();
                    ballocInfo.descriptorSetCount = static_cast<uint32_t>(m_layouts.size() - blayoutIndex);
                    ballocInfo.pSetLayouts = &m_layouts[blayoutIndex];

                    uint32_t counts[1];
                    counts[0] = static_cast<uint32_t>(buav->size());
                    VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts = {};
                    set_counts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
                    set_counts.descriptorSetCount = 1;
                    set_counts.pDescriptorCounts = counts;
                    ballocInfo.pNext = &set_counts;

                    res = vkAllocateDescriptorSets(static_cast<DeviceImplVulkan*>(m_device.native())->device(), &ballocInfo, &currentDescriptorSets[bindlessSet]);
                    ASSERT(res == VK_SUCCESS);


                    auto currentCount = resourceViews.count_textureUavs;
                    for (int i = 0; i < buav->size(); ++i)
                    {
                        auto tex = buav->get(i);
                        VkDescriptorImageInfo imageInfo = {};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        if (tex.valid())
                            imageInfo.imageView = static_cast<TextureUAVImplVulkan*>(tex.m_impl)->native();
                        else
                            imageInfo.imageView = static_cast<TextureUAVImplVulkan*>(m_device.nullResouces().textureUAV.resource().m_impl)->native();
                        imageInfo.sampler = VK_NULL_HANDLE;

                        resourceViews.textureUavs[resourceViews.count_textureUavs] = imageInfo;
                        ++resourceViews.count_textureUavs;
                    }
                    auto resCount = resourceViews.count_textureUavs - currentCount;
                    if (resCount > 0)
                    {
                        VkWriteDescriptorSet desc = {};
                        desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        desc.dstSet = currentDescriptorSets[bindlessSet];
                        desc.dstBinding = 0;
                        desc.dstArrayElement = 0;
                        desc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        desc.descriptorCount = static_cast<uint32_t>(resCount);
                        desc.pImageInfo = &resourceViews.textureUavs[currentCount];
                        descriptorWrites.emplace_back(desc);
                    }

                    ++bindlessSet;
                }
                for (size_t i = 0; i < shader->bindless_buffer_srvs().size(); ++i) { ASSERT(false, "not implemented yet"); }
                for (size_t i = 0; i < shader->bindless_buffer_uavs().size(); ++i) { ASSERT(false, "not implemented yet"); }
            };

            currentDescriptorSets.resize(countPipelineSets(configuration));
            if (configuration->hasVertexShader())
            {
                gatherSets(
                    configuration->vertexShader(), 
                    configuration->vertexShader()->setStartIndex(),
                    configuration->vertexShader()->setCount());
            }
            if (configuration->hasGeometryShader())
            {
                gatherSets(
                    configuration->geometryShader(), 
                    configuration->geometryShader()->setStartIndex(),
                    configuration->geometryShader()->setCount());
            }
            if (configuration->hasHullShader())
            {
                gatherSets(
                    configuration->hullShader(), 
                    configuration->hullShader()->setStartIndex(),
                    configuration->hullShader()->setCount());
            }
            if (configuration->hasDomainShader())
            {
                gatherSets(
                    configuration->domainShader(), 
                    configuration->domainShader()->setStartIndex(),
                    configuration->domainShader()->setCount());
            }
            if (configuration->hasPixelShader())
            {
                gatherSets(
                    configuration->pixelShader(), 
                    configuration->pixelShader()->setStartIndex(),
                    configuration->pixelShader()->setCount());
            }
            if (configuration->hasComputeShader())
            {
                gatherSets(
                    configuration->computeShader(), 
                    configuration->computeShader()->setStartIndex(),
                    configuration->computeShader()->setCount());
            }

            vkUpdateDescriptorSets(
                static_cast<DeviceImplVulkan*>(m_device.native())->device(),
                static_cast<uint32_t>(descriptorWrites.size()), 
                descriptorWrites.data(), 
                0, nullptr);
        }

        void PipelineImplVulkan::createGraphicsPipeline()
        {
            VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
            VkPipelineDynamicStateCreateInfo dynamicState = {};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = 3;
            dynamicState.pDynamicStates = dynamicStates;

            auto typeStringToType = [](const engine::string& typeString)->VkFormat
            {
                if (typeString == "uint")
                    return VK_FORMAT_R32_UINT;
                else if (typeString == "float2")
                    return VK_FORMAT_R32G32_SFLOAT;
                else
                    ASSERT(false, "nope");
                return VK_FORMAT_R32_UINT;
            };

            auto createInputLayout = [&]()
            {
                m_bindings.clear();
                VkVertexInputBindingDescription bindingDesc;
                bindingDesc.binding = 0;
                bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                bindingDesc.stride = 0;
                

                m_bindingAttributes.clear();
                if (m_configuration->hasVertexShader())
                {
                    size_t offset = 0ull;
                    auto inputLayout = m_configuration->vertexShader()->inputParameters(m_configuration->vertexShader()->currentPermutationId());
                    unsigned int location = 0;
                    for (auto&& inputParameter : inputLayout)
                    {
                        VkVertexInputAttributeDescription attribute;
                        attribute.binding = bindingDesc.binding;
                        attribute.format = typeStringToType(inputParameter.type);
                        attribute.location = location++;
                        attribute.offset = static_cast<uint32_t>(offset);

                        offset += formatBytes(fromVulkanFormat(attribute.format));

                        m_bindingAttributes.emplace_back(attribute);
                    }

                    bindingDesc.stride = static_cast<uint32_t>(offset);
                }
                m_bindings.emplace_back(bindingDesc);
            };
            if(m_useInputLaytout)
                createInputLayout();

			if (m_configuration->hasComputeShader())
			{
				VkComputePipelineCreateInfo pipelineInfo = {};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
				pipelineInfo.layout = *m_pipelineLayout;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
				pipelineInfo.basePipelineIndex = -1;

				pipelineInfo.stage = *m_computeShaderStageInfo;

				auto result = vkCreateComputePipelines(
                    static_cast<DeviceImplVulkan*>(m_device.native())->device(),
					VK_NULL_HANDLE, // VkPipelineCache object
					1,
					&pipelineInfo,
					nullptr,
                    m_currentPipelineCache->m_pipeline.get());
				ASSERT(result == VK_SUCCESS);
			}
			else
			{
				VkGraphicsPipelineCreateInfo pipelineInfo = {};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

				engine::vector<VkPipelineShaderStageCreateInfo> shaderStages;
				{
					if (m_configuration->hasVertexShader()) shaderStages.emplace_back(*m_vertShaderStageInfo.get());
					if (m_configuration->hasPixelShader()) shaderStages.emplace_back(*m_fragShaderStageInfo.get());
					if (m_configuration->hasGeometryShader()) shaderStages.emplace_back(*m_geometryShaderStageInfo.get());
					if (m_configuration->hasComputeShader()) shaderStages.emplace_back(*m_computeShaderStageInfo.get());
					if (m_configuration->hasDomainShader()) shaderStages.emplace_back(*m_domainShaderStageInfo.get());
					if (m_configuration->hasHullShader()) shaderStages.emplace_back(*m_hullShaderStageInfo.get());
				}
				pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
				pipelineInfo.pStages = shaderStages.data();

                // void PipelineImplVulkan::setInputLayout(unsigned int numElements, const InputElementDescription* inputElementDescs)


				VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
				{
					vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
					vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_bindings.size());
					vertexInputInfo.pVertexBindingDescriptions = m_bindings.size() > 0 ? m_bindings.data() : nullptr;
					vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_bindingAttributes.size());
					vertexInputInfo.pVertexAttributeDescriptions = m_bindingAttributes.size() > 0 ? m_bindingAttributes.data() : nullptr;
				}
				pipelineInfo.pVertexInputState = &vertexInputInfo;


				pipelineInfo.pInputAssemblyState = &m_inputAssembly;

				m_viewport = {};
				m_scissor = {};

				VkPipelineViewportStateCreateInfo viewportState = {};
				viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewportState.viewportCount = 1;
				viewportState.pViewports = &m_viewport;
				viewportState.scissorCount = 1;
				viewportState.pScissors = &m_scissor;
				pipelineInfo.pViewportState = &viewportState;

				VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
				for (auto&& rtv : m_renderPass->rtvs())
				{
					auto rtv_samples = rtv.texture().samples();
					if (rtv_samples > 1)
						samples = vulkanSamples(rtv_samples);
				}

				m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				m_multisampling.sampleShadingEnable = VK_FALSE;
				m_multisampling.rasterizationSamples = samples;
				m_multisampling.minSampleShading = 1.0f;
				m_multisampling.pSampleMask = nullptr;
				m_multisampling.alphaToCoverageEnable = VK_FALSE;
				m_multisampling.alphaToOneEnable = VK_FALSE;

				pipelineInfo.pRasterizationState = &m_rasterizer;
				pipelineInfo.pMultisampleState = &m_multisampling;

				if (m_depthBufferView)
					pipelineInfo.pDepthStencilState = &m_depthStencilCreateInfo;
				else
					pipelineInfo.pDepthStencilState = nullptr;

				resolveRTVDSVBlend();

				if (m_renderPass->dsvs().size() > 0)
				{
					pipelineInfo.pDepthStencilState = &m_depthStencilCreateInfo;
				}

				pipelineInfo.pColorBlendState = &m_colorBlending;
				pipelineInfo.pDynamicState = &dynamicState;
				pipelineInfo.layout = *m_pipelineLayout;
				pipelineInfo.renderPass = m_renderPass->renderPass();
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
				pipelineInfo.basePipelineIndex = -1;

				auto result = vkCreateGraphicsPipelines(
                    static_cast<DeviceImplVulkan*>(m_device.native())->device(),
					VK_NULL_HANDLE, // VkPipelineCache object
					1,
					&pipelineInfo,
					nullptr,
                    m_currentPipelineCache->m_pipeline.get());
				ASSERT(result == VK_SUCCESS);
			}
        }

        std::size_t PipelineImplVulkan::countShaderResources(const shaders::Shader* shader, bool _bindless, int bindlessIndex)
        {
            std::size_t res = 0;
            if (!_bindless)
            {
                res += const_cast<shaders::Shader*>(shader)->constants().size();
                res += shader->texture_srvs().size();
                res += shader->texture_uavs().size();
                res += shader->buffer_srvs().size();
                for (auto& uav : shader->buffer_uavs())
                    res += uav.valid() ? (uav.desc().append ? 2 : 1) : 1; // DirectXShaderCompiler creates a counter buffer binding automatically for append buffers
                res += shader->samplers().size();
                res += shader->root_constants().size();
            }
            else
            {
                int tempBindlessIndex = 0;
                for (auto& bindless : shader->bindless_texture_srvs())
                {
                    if (tempBindlessIndex == bindlessIndex)
                        return std::max(static_cast<int>(bindless->size()), 1);
                    ++tempBindlessIndex;
                }
                for (auto& bindless : shader->bindless_texture_uavs())
                {
                    if (tempBindlessIndex == bindlessIndex)
                        return std::max(static_cast<int>(bindless->size()), 1);
                    ++tempBindlessIndex;
                }
                for (auto& bindless : shader->bindless_buffer_srvs())
                {
                    if (tempBindlessIndex == bindlessIndex)
                        return std::max(static_cast<int>(bindless->size()), 1);
                    ++tempBindlessIndex;
                }
                for (auto& bindless : shader->bindless_buffer_uavs())
                {
                    if (tempBindlessIndex == bindlessIndex)
                        return std::max(static_cast<int>(bindless->size()), 1);
                    ++tempBindlessIndex;
                }
            }
            
            return res;
        }

        void PipelineImplVulkan::createRootSignature()
        {
            m_rootSignatures.clear();
            if (m_configuration->hasVertexShader())
            {
                for(int i = 0; i < m_configuration->vertexShader()->setCount(); ++i)
                    m_rootSignatures.emplace_back(engine::make_shared<RootSignature>(m_device.createRootSignature()));
            }
            if (m_configuration->hasGeometryShader())
            {
                for (int i = 0; i < m_configuration->geometryShader()->setCount(); ++i)
                    m_rootSignatures.emplace_back(engine::make_shared<RootSignature>(m_device.createRootSignature()));
            }
            if (m_configuration->hasHullShader())
            {
                for (int i = 0; i < m_configuration->hullShader()->setCount(); ++i)
                    m_rootSignatures.emplace_back(engine::make_shared<RootSignature>(m_device.createRootSignature()));
            }
            if (m_configuration->hasDomainShader())
            {
                for (int i = 0; i < m_configuration->domainShader()->setCount(); ++i)
                    m_rootSignatures.emplace_back(engine::make_shared<RootSignature>(m_device.createRootSignature()));
            }
            if (m_configuration->hasPixelShader())
            {
                for (int i = 0; i < m_configuration->pixelShader()->setCount(); ++i)
                    m_rootSignatures.emplace_back(engine::make_shared<RootSignature>(m_device.createRootSignature()));
            }
            if (m_configuration->hasComputeShader())
            {
                for (int i = 0; i < m_configuration->computeShader()->setCount(); ++i)
                    m_rootSignatures.emplace_back(engine::make_shared<RootSignature>(m_device.createRootSignature()));
            }

            /*

            typedef enum VkDescriptorType {
                VK_DESCRIPTOR_TYPE_SAMPLER = 0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
                
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,				    Texture SRV
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,				    Texture UAV
                
                VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,			Buffer SRV typed
                VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,			Buffer UAV typed
                
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,				    Buffer SRV structured
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,				    Buffer UAV structured

                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
                VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
                VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT = 1000138000,
                VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR = 1000150000,
                VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
                VK_DESCRIPTOR_TYPE_MUTABLE_VALVE = 1000351000,
                VK_DESCRIPTOR_TYPE_MAX_ENUM = 0x7FFFFFFF
            } VkDescriptorType;

            */

            auto bindResources = [&](
                RootSignature& rootSignature, 
                const shaders::Shader* shader,
                uint32_t& currentIndex,
                ShaderVisibility visibility,
                bool bindless,
                int bindlessIndex)
            {
                if (!bindless)
                {
                    uint32_t appendCounterBuffers = 0;
                    for (int i = 0; i < const_cast<shaders::Shader*>(shader)->root_constants().size(); ++i)
                    {
                        rootSignature[currentIndex].binding(currentIndex);
                        static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::UniformBuffer);
                        rootSignature[currentIndex].visibility(visibility);
                        ++currentIndex;
                    }
                    for (int i = 0; i < shader->samplers().size(); ++i)
                    {
                        rootSignature[currentIndex].binding(currentIndex);
                        static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::Sampler);
                        rootSignature[currentIndex].visibility(visibility);
                        ++currentIndex;
                    }
                    for (int i = 0; i < shader->texture_srvs().size(); ++i)
                    {
                        rootSignature[currentIndex].binding(currentIndex);
                        static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::SampledImage);
                        rootSignature[currentIndex].visibility(visibility);
                        ++currentIndex;
                    }
                    for (int i = 0; i < shader->texture_uavs().size(); ++i)
                    {
                        rootSignature[currentIndex].binding(currentIndex);
                        static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::StorageImage);
                        rootSignature[currentIndex].visibility(visibility);
                        ++currentIndex;
                    }
                    for (int i = 0; i < shader->buffer_srvs().size(); ++i)
                    {
                        if (shader->buffer_srvs_is_structured()[i])
                        {
                            rootSignature[currentIndex].binding(currentIndex);
                            static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::StorageBuffer);
                            rootSignature[currentIndex].visibility(visibility);
                            ++currentIndex;
                        }
                    }
                    for (int i = 0; i < shader->buffer_srvs().size(); ++i)
                    {
                        if (!shader->buffer_srvs_is_structured()[i])
                        {
                            rootSignature[currentIndex].binding(currentIndex);
                            static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::UniformTexelBuffer);
                            rootSignature[currentIndex].visibility(visibility);
                            ++currentIndex;
                        }
                    }
                    for (int i = 0; i < shader->buffer_uavs().size(); ++i)
                    {
                        if (shader->buffer_uavs_is_structured()[i])
                        {
                            rootSignature[currentIndex].binding(currentIndex);
                            static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::StorageBuffer);
                            rootSignature[currentIndex].visibility(visibility);
                            ++currentIndex;

                            if (shader->buffer_uavs()[i].valid() && shader->buffer_uavs()[i].desc().append)
                                ++appendCounterBuffers;
                        }
                    }
                    for (int i = 0; i < shader->buffer_uavs().size(); ++i)
                    {
                        if (!shader->buffer_uavs_is_structured()[i])
                        {
                            rootSignature[currentIndex].binding(currentIndex);
                            static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::StorageTexelBuffer);
                            rootSignature[currentIndex].visibility(visibility);
                            ++currentIndex;

                            if (shader->buffer_uavs()[i].valid() && shader->buffer_uavs()[i].desc().append)
                                ++appendCounterBuffers;
                        }
                    }
                    for (int i = 0; i < const_cast<shaders::Shader*>(shader)->constants().size(); ++i)
                    {
                        rootSignature[currentIndex].binding(currentIndex);
                        static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::UniformBuffer);
                        rootSignature[currentIndex].visibility(visibility);
                        ++currentIndex;
                    }

                    for (uint32_t i = 0; i < appendCounterBuffers; ++i)
                    {
                        rootSignature[currentIndex].binding(currentIndex);
                        static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::StorageBuffer);
                        rootSignature[currentIndex].visibility(visibility);
                        ++currentIndex;
                    }
                }
                else
                {
                    int currentBindlessIndex = 0;
                    for (auto&& res : shader->bindless_texture_srvs())
                    {
                        if (currentBindlessIndex == bindlessIndex)
                        {
                            if (res->size() == 0)
                            {
                                for(int i = 0; i < 10; ++i)
                                    const_cast<BindlessTextureSRV*>(res)->push(m_device.nullResouces().textureSRV);
                            }

                            for (int i = 0; i < res->size(); ++i)
                            {
                                rootSignature[currentIndex].binding(currentIndex);
                                static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::SampledImage);
                                rootSignature[currentIndex].visibility(visibility);
                                ++currentIndex;
                            }
                            return;
                        }
                        ++currentBindlessIndex;
                    }
                    for (auto&& res : shader->bindless_buffer_srvs())
                    {
                        if (currentBindlessIndex == bindlessIndex)
                        {
                            for (int i = 0; i < res->size(); ++i)
                            {
                                rootSignature[currentIndex].binding(currentIndex);
                                static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::UniformTexelBuffer);
                                rootSignature[currentIndex].visibility(visibility);
                                ++currentIndex;
                            }
                            return;
                        }
                        ++currentBindlessIndex;
                    }
                    for (auto&& res : shader->bindless_texture_uavs())
                    {
                        if (currentBindlessIndex == bindlessIndex)
                        {
                            for (int i = 0; i < res->size(); ++i)
                            {
                                rootSignature[currentIndex].binding(currentIndex);
                                static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::StorageImage);
                                rootSignature[currentIndex].visibility(visibility);
                                ++currentIndex;
                            }
                            return;
                        }
                        ++currentBindlessIndex;
                    }
                    for (auto&& res : shader->bindless_buffer_uavs())
                    {
                        if (currentBindlessIndex == bindlessIndex)
                        {
                            for (int i = 0; i < res->size(); ++i)
                            {
                                rootSignature[currentIndex].binding(currentIndex);
                                static_cast<RootParameterImplVulkan*>(rootSignature[currentIndex].native())->descriptorType(DescriptorType::StorageBuffer);
                                rootSignature[currentIndex].visibility(visibility);
                                ++currentIndex;
                            }
                            return;
                        }
                        ++currentBindlessIndex;
                    }
                }
				
            };

            auto createDescriptorSet = [&](const shaders::Shader* shader, ShaderVisibilityBits visibility, int setIndex, int setCount)
            {
                RootSignature& rootSignature = *m_rootSignatures[setIndex];
                rootSignature.reset(static_cast<int>(countShaderResources(shader, false, 0)), 0);
                uint32_t currentIndex = 0;
                bindResources(rootSignature, shader, currentIndex, static_cast<ShaderVisibility>(visibility), false, 0);
                rootSignature.finalize();

                // bindless
                /*for(int i = 1; i < setCount; ++i)
                {
                    RootSignature& rs = *m_rootSignatures[setIndex+i];
                    rs.reset(static_cast<int>(countShaderResources(shader, true, i-1)), 0);
                    uint32_t currentIndex = 0;

                    bindResources(rs, shader, currentIndex, static_cast<ShaderVisibility>(visibility), true, i-1);
                    //rootSignature.enableNullDescriptors(true);
                    rs.finalize();
                }*/

                auto allBindlessCount =
                    shader->bindless_texture_srvs().size() +
                    shader->bindless_texture_uavs().size() +
                    shader->bindless_buffer_srvs().size() +
                    shader->bindless_buffer_uavs().size();

                // setCount == SRV/UAV/CBV set + bindless sets. (it's also possible there are no SRV/UAV/CBV set)
                size_t bindlessSet = setIndex + (setCount - allBindlessCount);

                for (int i = 0; i < shader->bindless_texture_srvs().size(); ++i)
                {
                    RootSignature& rs = *m_rootSignatures[bindlessSet];
                    rs.reset(1, 0);

                    rs[0].binding(0);
                    static_cast<RootParameterImplVulkan*>(rs[0].native())->descriptorType(DescriptorType::SampledImage);
                    rs[0].visibility(static_cast<ShaderVisibility>(visibility));

                    rs.enableNullDescriptors(true, false);
                    static_cast<RootSignatureImplVulkan*>(rs.native())->setNextVisibility(static_cast<ShaderVisibility>(visibility));
                    rs.finalize();

                    ++bindlessSet;
                }
                for (int i = 0; i < shader->bindless_buffer_srvs().size(); ++i)
                {
                    RootSignature& rs = *m_rootSignatures[bindlessSet];
                    rs.reset(1, 0);

                    rs[0].binding(0);
                    static_cast<RootParameterImplVulkan*>(rs[0].native())->descriptorType(DescriptorType::UniformTexelBuffer);
                    rs[0].visibility(static_cast<ShaderVisibility>(visibility));

                    rs.enableNullDescriptors(false, false);
                    static_cast<RootSignatureImplVulkan*>(rs.native())->setNextVisibility(static_cast<ShaderVisibility>(visibility));
                    rs.finalize();

                    ++bindlessSet;
                }
                for (int i = 0; i < shader->bindless_texture_uavs().size(); ++i)
                {
                    RootSignature& rs = *m_rootSignatures[bindlessSet];
                    rs.reset(1, 0);

                    rs[0].binding(0);
                    static_cast<RootParameterImplVulkan*>(rs[0].native())->descriptorType(DescriptorType::StorageImage);
                    rs[0].visibility(static_cast<ShaderVisibility>(visibility));

                    rs.enableNullDescriptors(true, true);
                    static_cast<RootSignatureImplVulkan*>(rs.native())->setNextVisibility(static_cast<ShaderVisibility>(visibility));
                    rs.finalize();

                    ++bindlessSet;
                }
                for (int i = 0; i < shader->bindless_buffer_uavs().size(); ++i)
                {
                    RootSignature& rs = *m_rootSignatures[bindlessSet];
                    rs.reset(1, 0);

                    rs[0].binding(0);
                    static_cast<RootParameterImplVulkan*>(rs[0].native())->descriptorType(DescriptorType::StorageBuffer);
                    rs[0].visibility(static_cast<ShaderVisibility>(visibility));

                    rs.enableNullDescriptors(false, true);
                    static_cast<RootSignatureImplVulkan*>(rs.native())->setNextVisibility(static_cast<ShaderVisibility>(visibility));
                    rs.finalize();

                    ++bindlessSet;
                }
                
            };


            if (m_configuration->hasVertexShader())
            {
                createDescriptorSet(
                    m_configuration->vertexShader(), 
                    ShaderVisibilityBits::Vertex, 
                    m_configuration->vertexShader()->setStartIndex(),
                    m_configuration->vertexShader()->setCount());
            }
            if (m_configuration->hasGeometryShader())
            {
                createDescriptorSet(
                    m_configuration->geometryShader(), 
                    ShaderVisibilityBits::Geometry, 
                    m_configuration->geometryShader()->setStartIndex(),
                    m_configuration->geometryShader()->setCount());
            }
            if (m_configuration->hasHullShader())
            {
                createDescriptorSet(
                    m_configuration->hullShader(), 
                    ShaderVisibilityBits::Hull, 
                    m_configuration->hullShader()->setStartIndex(),
                    m_configuration->hullShader()->setCount());
            }
            if (m_configuration->hasDomainShader())
            {
                createDescriptorSet(
                    m_configuration->domainShader(),
                    ShaderVisibilityBits::Domain, 
                    m_configuration->domainShader()->setStartIndex(),
                    m_configuration->domainShader()->setCount());
            }
            if (m_configuration->hasPixelShader())
            {
                createDescriptorSet(
                    m_configuration->pixelShader(), 
                    ShaderVisibilityBits::Pixel, 
                    m_configuration->pixelShader()->setStartIndex(),
                    m_configuration->pixelShader()->setCount());
            }
            if (m_configuration->hasComputeShader())
            {
                createDescriptorSet(
                    m_configuration->computeShader(), 
                    ShaderVisibilityBits::Compute, 
                    m_configuration->computeShader()->setStartIndex(),
                    m_configuration->computeShader()->setCount());
            }


            /*rootSignature[0].binding(0);
            rootSignature[0].descriptorType(DescriptorType::UniformBuffer);
            rootSignature[0].visibility(static_cast<ShaderVisibility>(ShaderVisibilityBits::Vertex));
            rootSignature[1].binding(1);
            rootSignature[1].descriptorType(DescriptorType::CombinedImageSampler);
            rootSignature[1].visibility(static_cast<ShaderVisibility>(ShaderVisibilityBits::Pixel));*/
            
            setRootSignature();
#if 0
            InputElementDescription vertDesc;
            vertDesc.alignedByteOffset(sizeof(Vertex))
                .format(Format::R32G32B32_FLOAT)
                .inputSlot(0)
                .inputSlotClass(InputClassification::PerVertexData)
                .offset(offsetof(Vertex, pos));

            InputElementDescription colorDesc;
            colorDesc.alignedByteOffset(sizeof(Vertex))
                .format(Format::R32G32B32_FLOAT)
                .inputSlot(0)
                .inputSlotClass(InputClassification::PerVertexData)
                .offset(offsetof(Vertex, color));

            InputElementDescription uvDesc;
            uvDesc.alignedByteOffset(sizeof(Vertex))
                .format(Format::R32G32_FLOAT)
                .inputSlot(0)
                .inputSlotClass(InputClassification::PerVertexData)
                .offset(offsetof(Vertex, texCoord));

            m_pipelineDescriptions.emplace_back(vertDesc);
            m_pipelineDescriptions.emplace_back(colorDesc);
            m_pipelineDescriptions.emplace_back(uvDesc);

            setInputLayout(static_cast<unsigned int>(
                m_pipelineDescriptions.size()),
                m_pipelineDescriptions.data());

            /*if (m_vertexShader) setVertexShader(*m_vertexShader);
            if (m_pixelShader) setPixelShader(*m_pixelShader);
            if (m_geometryShader) setGeometryShader(*m_geometryShader);
            if (m_hullShader) setHullShader(*m_hullShader);
            if (m_domainShader) setDomainShader(*m_domainShader);
            if (m_computeShader) setComputeShader(*m_computeShader);*/


            m_rootSignature = engine::make_shared<RootSignature>(m_device.createRootSignature());
            RootSignature& rootSignature = *m_rootSignature;
            rootSignature.reset(2, 0);
            rootSignature[0].binding(0);
            rootSignature[0].descriptorType(DescriptorType::UniformBuffer);
            rootSignature[0].visibility(static_cast<ShaderVisibility>(ShaderVisibilityBits::Vertex));
            rootSignature[1].binding(1);
            rootSignature[1].descriptorType(DescriptorType::CombinedImageSampler);
            rootSignature[1].visibility(static_cast<ShaderVisibility>(ShaderVisibilityBits::Pixel));
            rootSignature.finalize();
            setRootSignature(rootSignature);
#endif
        }

        void PipelineImplVulkan::setUniformBuffer(const Buffer& buffer)
        {
            m_uniformBuffers.emplace_back(std::move(buffer));
        }

        void PipelineImplVulkan::setTextureSRV(const TextureSRV& srv)
        {
            m_srvs.emplace_back(srv);
        }

        void PipelineImplVulkan::setSampler(const Sampler& sampler)
        {
            m_samplers.emplace_back(sampler);
        }
    }
}
