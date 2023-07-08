#include "engine/rendering/tools/GaussianBlurTool.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    GaussianBlurTool::GaussianBlurTool(Device& device)
        : m_device{ device }
        , m_gaussianBlurPipeline{ device.createPipeline<shaders::GaussianBlur>() }
    {
        m_gaussianBlurPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_gaussianBlurPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_gaussianBlurPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_gaussianBlurPipeline.ps.imageSampler = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));
    }

    void GaussianBlurTool::blur(CommandList& cmd, TextureSRV src, TextureSRV target)
    {
        CPU_MARKER(cmd.api(), "Gaussian blur");
        GPU_MARKER(cmd, "Gaussian blur");

        auto containerIndex = refreshInternalTextures(src);

        cmd.copyTexture(src, m_blurTextures[containerIndex].m_blurTargetSRV[0]);

        for (int i = 0; i < 6; ++i)
        {
            // blur the bright target
            m_gaussianBlurPipeline.ps.image = m_blurTextures[containerIndex].m_blurTargetSRV[i % 2];
            m_gaussianBlurPipeline.ps.width = static_cast<float>(src.width());
            m_gaussianBlurPipeline.ps.height = static_cast<float>(src.height());
            m_gaussianBlurPipeline.ps.horizontal.x = i % 2;

            cmd.setRenderTargets({ m_blurTextures[containerIndex].m_blurTargetRTV[(i+1) % 2] });
            cmd.bindPipe(m_gaussianBlurPipeline);
            cmd.draw(4u);
        }
        
        cmd.copyTexture(m_blurTextures[containerIndex].m_blurTargetSRV[0], target);
    }

    void GaussianBlurTool::blur(CommandList& cmd, TextureSRV src, TextureUAV target)
    {
        CPU_MARKER(cmd.api(), "Gaussian blur");
        GPU_MARKER(cmd, "Gaussian blur");

        auto containerIndex = refreshInternalTextures(src);

        cmd.copyTexture(src, m_blurTextures[containerIndex].m_blurTargetUAV[0]);

        for (int i = 0; i < 6; ++i)
        {
            // blur the bright target
            m_gaussianBlurPipeline.ps.image = m_blurTextures[containerIndex].m_blurTargetSRV[i % 2];
            m_gaussianBlurPipeline.ps.width = static_cast<float>(src.width());
            m_gaussianBlurPipeline.ps.height = static_cast<float>(src.height());
            m_gaussianBlurPipeline.ps.horizontal.x = i % 2;

            cmd.setRenderTargets({ m_blurTextures[containerIndex].m_blurTargetRTV[(i + 1) % 2] });
            cmd.bindPipe(m_gaussianBlurPipeline);
            cmd.draw(4u);
        }

        cmd.copyTexture(m_blurTextures[containerIndex].m_blurTargetSRV[0], target);
    }

    int GaussianBlurTool::refreshInternalTextures(TextureSRV src)
    {
        for(int i = 0; i < m_blurTextures.size(); ++i)
        {
            if (m_blurTextures[i].width == src.width() && m_blurTextures[i].height == src.height())
            {
                return i;
            }
        }

        BlurContainer container;
        container.width = src.width();
        container.height = src.height();
        for (int i = 0; i < 2; ++i)
        {
            container.m_blurTargetRTV[i] = m_device.createTextureRTV(TextureDescription()
                .width(src.width())
                .height(src.height())
                .mipLevels(4)
                .format(src.format())
                .usage(ResourceUsage::GpuRenderTargetReadWrite)
                .name("Gaussian blur target")
                .dimension(ResourceDimension::Texture2D)
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));
            container.m_blurTargetSRV[i] = m_device.createTextureSRV(container.m_blurTargetRTV[i]);
            container.m_blurTargetUAV[i] = m_device.createTextureUAV(container.m_blurTargetRTV[i]);
        }
        int res = static_cast<int>(m_blurTextures.size());
        m_blurTextures.emplace_back(std::move(container));
        return res;
    }
}
