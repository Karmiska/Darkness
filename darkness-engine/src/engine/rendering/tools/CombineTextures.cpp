#include "engine/rendering/tools/CombineTextures.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    CombineTextures::CombineTextures(Device& device)
        : m_device{ device }
        , m_addTexturesPipeline{ device.createPipeline<shaders::AddTextures>() }
    {
        m_addTexturesPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_addTexturesPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_addTexturesPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_addTexturesPipeline.ps.samp = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));
    }

    void CombineTextures::combine(CommandList& cmd, TextureSRV textureA, TextureSRV textureB, TextureRTV dst)
    {
        CPU_MARKER(cmd.api(), "Combine textures");
        GPU_MARKER(cmd, "Combine textures");

        m_addTexturesPipeline.ps.texturea = textureA;
        m_addTexturesPipeline.ps.textureb = textureB;

        cmd.setRenderTargets({ dst });
        cmd.bindPipe(m_addTexturesPipeline);
        cmd.draw(4u);
    }
}
