#include "engine/rendering/Postprocess.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Device.h"
#include "tools/Measure.h"

using namespace tools;

constexpr uint32_t BlurCount = 2;

namespace engine
{
    Postprocess::Postprocess(Device& device)
        : m_device{ device }
        , m_exposure{ device }
        , m_bloomAndLuminance{ device }
        , m_tonemap{ device }
        , m_histogram{ device }
        , m_adaptiveExposure{ device }
        /*, m_blur{ device }
        , m_downsample{ device }
        , m_extractBrightness{ device }
        , m_combineTextures{ device }
        , m_bloomPipeline{ device.createPipeline<shaders::Bloom>() }
        , m_tonemapPipeline{ device.createPipeline<shaders::Tonemap>() }
        , m_bloomPipelineHDR{ device.createPipeline<shaders::Bloom>() }
        , m_tonemapPipelineHDR{ device.createPipeline<shaders::Tonemap>() }
        , m_copyTexture{ device.createPipeline<shaders::CopyTexture>() }
        , m_averageLuminance{ device.createPipeline<shaders::AverageLuminance>() }
        , m_lastFrameWidth{ 0u }
        , m_lastFrameHeight{ 0u }
        , m_lastLuminanceWidth{ 0u }*/
    {
        /*m_copyTexture.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_copyTexture.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_copyTexture.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_copyTexture.ps.samp = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));

        // normal pipe
        {
            m_bloomPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
            m_bloomPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
            m_bloomPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
            m_bloomPipeline.ps.framebufferSampler = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));

            m_tonemapPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
            m_tonemapPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
            m_tonemapPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
            m_tonemapPipeline.ps.imageSampler = device.createSampler(SamplerDescription().filter(engine::Filter::Point));
        }

        // hdr pipe
        {
            m_bloomPipelineHDR.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
            m_bloomPipelineHDR.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
            m_bloomPipelineHDR.setDepthStencilState(DepthStencilDescription().depthEnable(false));
            m_bloomPipelineHDR.ps.framebufferSampler = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));

            m_tonemapPipelineHDR.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
            m_tonemapPipelineHDR.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
            m_tonemapPipelineHDR.setDepthStencilState(DepthStencilDescription().depthEnable(false));
            m_tonemapPipelineHDR.ps.imageSampler = device.createSampler(SamplerDescription().filter(engine::Filter::Point));
        }*/
    }

    void Postprocess::render(
        TextureRTV currentRenderTarget,
        TextureSRV frame,
        CommandList& cmd,
        PostprocessComponent& postprocessComponent,
        bool histogramDebug)
    {
        CPU_MARKER(cmd.api(), "Postprocess");
        GPU_MARKER(cmd, "Postprocess");

        refreshInternalTextures(frame);

        auto postProcessSettings = postprocessComponent.settings();
        auto needExtraColorInput = postProcessSettings.chromaticAberration.enabled;

        auto sourceImageUAV = needExtraColorInput ? m_colorInputUAV : m_colorUAV;
        auto sourceImageSRV = needExtraColorInput ? m_colorInputSRV : m_colorSRV;

        {
            CPU_MARKER(cmd.api(), "Copy frame for postprocessing");
            GPU_MARKER(cmd, "Copy frame for postprocessing");

            
            cmd.copyTexture(frame, sourceImageUAV);
        }

        m_bloomAndLuminance.extract(
            cmd,
            sourceImageSRV,
            m_exposure,
            postProcessSettings.bloom.enabled,
            postProcessSettings.bloom.threshold);

        m_tonemap.tonemap(
            cmd,
            m_exposure.exposureSRV(),
            m_bloomAndLuminance.bloom(),
            sourceImageSRV,
            m_colorUAV,
            g_LumaBufferUAV, 
            postProcessSettings.bloom.strength,
            postProcessSettings.vignette,
            postProcessSettings.chromaticAberration);

        m_histogram.histogram(cmd, m_bloomAndLuminance.lumaLRSRV());

        if(histogramDebug)
            m_histogram.drawDebugHistogram(cmd, m_exposure.exposureSRV(), m_colorUAV);

        m_adaptiveExposure.adapt(
            cmd,
            m_histogram.histogramSRV(),
            m_exposure.exposureUAV(),
            postProcessSettings.adaptiveExposure.targetLuminance,
            postProcessSettings.adaptiveExposure.adaptationRate,
            postProcessSettings.adaptiveExposure.minExposure,
            postProcessSettings.adaptiveExposure.maxExposure,
            static_cast<uint32_t>(m_bloomAndLuminance.lumaLRSRV().width() * m_bloomAndLuminance.lumaLRSRV().height()));

        {
            CPU_MARKER(cmd.api(), "Copy tonemapped frame to backbuffer");
            GPU_MARKER(cmd, "Copy tonemapped frame to backbuffer");

            cmd.copyTexture(m_colorSRV, currentRenderTarget);
        }
    }

    void Postprocess::refreshInternalTextures(TextureSRV src)
    {
        if (!m_colorUAV.resource().valid() ||
            m_colorUAV.resource().width() != src.width() ||
            m_colorUAV.resource().height() != src.height())
        {
            m_colorUAV = m_device.createTextureUAV(TextureDescription()
                .width(src.width())
                .height(src.height())
                .format(Format::R11G11B10_FLOAT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("Color texture")
                .dimension(ResourceDimension::Texture2D));
            m_colorSRV = m_device.createTextureSRV(m_colorUAV);

            m_colorInputUAV = m_device.createTextureUAV(TextureDescription()
                .width(src.width())
                .height(src.height())
                .format(Format::R11G11B10_FLOAT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("Color input texture")
                .dimension(ResourceDimension::Texture2D));
            m_colorInputSRV = m_device.createTextureSRV(m_colorInputUAV);
        }

        if (!g_LumaBufferUAV.resource().valid() ||
            g_LumaBufferUAV.resource().width() != src.width() ||
            g_LumaBufferUAV.resource().height() != src.height())
        {
            g_LumaBufferUAV = m_device.createTextureUAV(TextureDescription()
                .width(src.width())
                .height(src.height())
                .format(Format::R8_UNORM)
                .usage(ResourceUsage::GpuReadWrite)
                .name("Luminance texture")
                .dimension(ResourceDimension::Texture2D));
            g_LumaBufferSRV = m_device.createTextureSRV(g_LumaBufferUAV);
        }
    }
}
