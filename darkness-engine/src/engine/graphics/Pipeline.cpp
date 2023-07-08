#include "engine/graphics/Pipeline.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Sampler.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12Pipeline.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanPipeline.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanSampler.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalPipeline.h"
#endif


using namespace tools;
using namespace engine::implementation;

namespace engine
{
    PipelineAbs::PipelineAbs(Device& device,
        ShaderStorage& storage,
        GraphicsApi api)
        : m_impl{}
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_shared<implementation::PipelineImplDX12>(device, storage);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_shared<implementation::PipelineImplVulkan>(device, storage);

    };

    void PipelineAbs::setBlendState(const BlendDescription& desc)
    {
        m_impl->setBlendState(desc);
    };

    void PipelineAbs::setRasterizerState(const RasterizerDescription& desc)
    {
        m_impl->setRasterizerState(desc);
    };

    void PipelineAbs::setDepthStencilState(const DepthStencilDescription& desc)
    {
        m_impl->setDepthStencilState(desc);
    };

    void PipelineAbs::setSampleMask(unsigned int mask)
    {
        m_impl->setSampleMask(mask);
    };
    
    void PipelineAbs::setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency)
    {
        m_impl->setPrimitiveTopologyType(type, adjacency);
    };

    void PipelineAbs::setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality) 
    {
        m_impl->setRenderTargetFormat(RTVFormat, DSVFormat, msaaCount, msaaQuality);
    };

    void PipelineAbs::setRenderTargetFormats(
        engine::vector<Format> RTVFormats, 
        Format DSVFormat, 
        unsigned int msaaCount, 
        unsigned int msaaQuality) 
    {
        m_impl->setRenderTargetFormats(RTVFormats, DSVFormat, msaaCount, msaaQuality);
    };

    void PipelineAbs::setPrimitiveRestart(IndexBufferStripCutValue value)
    {
        m_impl->setPrimitiveRestart(value);
    };

    void PipelineAbs::configure(CommandListImplIf* commandList, shaders::PipelineConfiguration* configuration)
    {
        m_impl->configure(commandList, configuration);
    };

}
