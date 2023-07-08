#pragma once

#include "engine/graphics/PipelineImplIf.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanRenderPass.h"
#include "engine/graphics/CommonNoDep.h"
#include "containers/memory.h"
#include "containers/vector.h"
#include "containers/string.h"

namespace engine
{
    namespace shaders
    {
        class PipelineConfiguration;
    }
    class Device;
    class TextureDSV;
    class TextureRTV;
    class BufferView;
    class Buffer;
    class RootSignature;
    struct BlendDescription;
    struct RasterizerDescription;
    struct DepthStencilDescription;
    enum class PrimitiveTopologyType;
    enum class Format;
    struct InputElementDescription;
    enum class IndexBufferStripCutValue;
    class ShaderBinary;
    class CommandList;
    class ShaderBinary;
    class Sampler;
    class TextureSRV;
    class ShaderStorage;

    namespace implementation
    {
        enum class DescriptorType
        {
            Sampler,
            CombinedImageSampler,
            SampledImage,
            StorageImage,
            UniformTexelBuffer,
            StorageTexelBuffer,
            UniformBuffer,
            StorageBuffer,
            UniformBufferDynamic,
            StorageBufferDynamic,
            InputAttachment
        };

        class CommandListImplVulkan;
        class DescriptorHandleImplVulkan;
        class PipelineImplVulkan : public PipelineImplIf
        {
        public:
            PipelineImplVulkan(
                Device& device,
                ShaderStorage& storage);
            
            PipelineImplVulkan(const PipelineImplVulkan&) = delete;
            PipelineImplVulkan(PipelineImplVulkan&&) = delete;
            PipelineImplVulkan& operator=(const PipelineImplVulkan&) = delete;
            PipelineImplVulkan& operator=(PipelineImplVulkan&&) = delete;
            
            void setBlendState(const BlendDescription& desc) override;
            void setRasterizerState(const RasterizerDescription& desc) override;
            void setDepthStencilState(const DepthStencilDescription& desc) override;
            void setSampleMask(unsigned int mask) override;
            void setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency = false) override;
            void setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) override;
            void setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) override;
            void setPrimitiveRestart(IndexBufferStripCutValue value) override;

            void setInputLayout(unsigned int numElements, const InputElementDescription* inputElementDescs);
            void setDepthBufferView(engine::shared_ptr<TextureDSV> view);

            void setUniformBuffer(const Buffer& buffer);
            void setTextureSRV(const TextureSRV& srv);
            void setSampler(const Sampler& sampler);

            void configure(
                CommandListImplIf* commandList, 
                shaders::PipelineConfiguration* configuration) override;

            void finalize(CommandListImplVulkan& cmd, shaders::PipelineConfiguration* configuration);
            
            const engine::vector<VkDescriptorSet>& descriptorSet() const;

            RenderPass& renderPass(shaders::PipelineConfiguration* configuration = nullptr);
        private:
            friend class CommandListImplVulkan;
            Device& m_device;
            ShaderStorage& m_storage;
            shaders::PipelineConfiguration* m_configuration;
            engine::shared_ptr<RenderPass> m_renderPass;
            bool m_finalized;
			BlendDescription m_blendDescription;
			engine::vector<Format> m_RTVFormats;
			Format m_DSVFormat;

            void setDefaults();
			void resolveRTVDSVBlend();

            // binary shaders
            void loadShaders(shaders::PipelineConfiguration* configuration);
            engine::shared_ptr<const ShaderBinary> m_vertexShader;
            engine::shared_ptr<const ShaderBinary> m_pixelShader;
            engine::shared_ptr<const ShaderBinary> m_geometryShader;
            engine::shared_ptr<const ShaderBinary> m_hullShader;
            engine::shared_ptr<const ShaderBinary> m_domainShader;
            engine::shared_ptr<const ShaderBinary> m_computeShader;
            void onShaderChange();

            // shader stages
            void setShaderStages(shaders::PipelineConfiguration* configuration);
            void setVertexShader(const ShaderBinary& shaderBinary);
            void setPixelShader(const ShaderBinary& shaderBinary);
            void setGeometryShader(const ShaderBinary& shaderBinary);
            void setHullShader(const ShaderBinary& shaderBinary);
            void setDomainShader(const ShaderBinary& shaderBinary);
            void setComputeShader(const ShaderBinary& shaderBinary);
            engine::unique_ptr<VkPipelineShaderStageCreateInfo> m_vertShaderStageInfo;
            engine::unique_ptr<VkPipelineShaderStageCreateInfo> m_fragShaderStageInfo;
            engine::unique_ptr<VkPipelineShaderStageCreateInfo> m_geometryShaderStageInfo;
            engine::unique_ptr<VkPipelineShaderStageCreateInfo> m_computeShaderStageInfo;
            engine::unique_ptr<VkPipelineShaderStageCreateInfo> m_domainShaderStageInfo;
            engine::unique_ptr<VkPipelineShaderStageCreateInfo> m_hullShaderStageInfo;

            void createRootSignature();
            void setRootSignature();

            int countPipelineSets(shaders::PipelineConfiguration* configuration);
            void createGraphicsPipeline();
            void createBindings(CommandListImplVulkan& cmd, shaders::PipelineConfiguration* configuration);

            VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
            VkViewport m_viewport;
            VkRect2D m_scissor;
            VkPipelineRasterizationStateCreateInfo m_rasterizer;
            VkPipelineMultisampleStateCreateInfo m_multisampling;
            VkPipelineColorBlendAttachmentState m_colorBlendAttachement[8];
            VkPipelineColorBlendStateCreateInfo m_colorBlending;

            VkPipelineLayoutCreateInfo m_pipelineLayoutInfo;
            VkPipelineDepthStencilStateCreateInfo m_depthStencilCreateInfo;

            engine::vector<VkVertexInputBindingDescription> m_bindings;
            engine::vector<VkVertexInputAttributeDescription> m_bindingAttributes;

            engine::shared_ptr<VkPipelineLayout> m_pipelineLayout;
            struct ConstantUpdate
            {
                BufferCBV buffer;
                tools::ByteRange range;
            };
            class PipelineCache
            {
            public:
                PipelineCache(const VkDevice& device);

                engine::shared_ptr<VkPipeline> m_pipeline;
                engine::vector<VkDescriptorSet> m_descriptorSets;
                engine::vector<ConstantUpdate> constantUploads;
            };
            engine::unordered_map<uint64_t, PipelineCache> m_hashResourceStorage;
            PipelineCache* m_currentPipelineCache;

            engine::shared_ptr<TextureDSV> m_depthBufferView;

            //engine::vector<engine::shared_ptr<VkFramebuffer>> m_framebuffers;
            //engine::vector<VkFramebuffer> m_framebuffers;
            engine::vector<VkDescriptorSetLayout> m_currentLayouts;

            engine::vector<InputElementDescription> m_pipelineDescriptions;
            struct PipelineInfo
            {
                engine::string name;
                DescriptorType type;
                ShaderVisibility visibility;
            };
            engine::vector<PipelineInfo> m_pipelineNames;

            //engine::shared_ptr<DescriptorHeap> m_descriptorHeap;
            engine::shared_ptr<DescriptorHandleImplVulkan> m_uniform;

            engine::vector<Buffer> m_uniformBuffers;
            engine::vector<TextureSRV> m_srvs;
            engine::vector<Sampler> m_samplers;

            engine::vector<engine::shared_ptr<RootSignature>> m_rootSignatures;

            std::size_t countShaderResources(const shaders::Shader* shader, bool bindless, int bindlessIndex);

            struct ResourceViews
            {
                engine::vector<VkDescriptorBufferInfo>	constants;
                engine::vector<VkDescriptorImageInfo>	textureSrvs;
                engine::vector<VkDescriptorImageInfo>	textureUavs;
                engine::vector<VkDescriptorBufferInfo>	bufferSrvsStructured;
                engine::vector<VkBufferView>			bufferSrvsTyped;
                engine::vector<VkDescriptorBufferInfo>	bufferUavsStructured;
                engine::vector<VkBufferView>			bufferUavsTyped;
                engine::vector<VkDescriptorImageInfo>	samplers;
                engine::vector<VkDescriptorBufferInfo>  bufferCounterUavs;

                engine::vector<VkDescriptorImageInfo>	bindlessTextureSrvs;
                engine::vector<VkDescriptorImageInfo>	bindlessTextureUavs;

                size_t count_constants = 0;
                size_t count_textureSrvs = 0;
                size_t count_textureUavs = 0;
                size_t count_bufferSrvsStructured = 0;
                size_t count_bufferSrvsTyped = 0;
                size_t count_bufferUavsStructured = 0;
                size_t count_bufferUavsTyped = 0;
                size_t count_samplers = 0;
                size_t count_bufferCounterUavs = 0;
            };
            ResourceViews m_resourceViews;
            engine::vector<VkDescriptorSetLayout> m_layouts;
            bool m_useInputLaytout;
        };
    }
}

