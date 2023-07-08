#include "engine/rendering/tools/DownsampleTool.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    DownsampleTool::DownsampleTool(Device& device)
        : m_device{ device }
        , m_downsamplePipeline{ device.createPipeline<shaders::Downsample>() }
    {
        m_downsamplePipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_downsamplePipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_downsamplePipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_downsamplePipeline.ps.imageSampler = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));
    }

    void DownsampleTool::downsample(CommandList& cmd, TextureSRV src, TextureRTV dst)
    {
        CPU_MARKER(cmd.api(), "Downsample");
        GPU_MARKER(cmd, "Downsample");

        m_downsamplePipeline.ps.image = src;

        cmd.setRenderTargets({ dst });
        cmd.bindPipe(m_downsamplePipeline);
        cmd.draw(4u);
    }
}
