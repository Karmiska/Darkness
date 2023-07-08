#include "engine/rendering/postprocessing/Tonemapper.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/SamplerDescription.h"

namespace engine
{
    Tonemapper::Tonemapper(Device& device)
        : m_tonemapDefaultHDR{ device.createPipeline<shaders::TonemapHDR>() }
        , m_tonemapVignetteHDR{ device.createPipeline<shaders::TonemapHDR>() }
        , m_tonemapChromaticAberrationHDR{ device.createPipeline<shaders::TonemapHDR>() }
        , m_tonemapVignetteChromaticAberrationHDR{ device.createPipeline<shaders::TonemapHDR>() }
    {
        m_tonemapDefaultHDR.cs.tonemap = shaders::TonemapHDRCS::Tonemap::Default;
        m_tonemapDefaultHDR.cs.LinearSampler = device.createSampler(SamplerDescription()
            .textureAddressMode(TextureAddressMode::Wrap)
            .filter(Filter::Point));
        m_tonemapVignetteHDR.cs.tonemap = shaders::TonemapHDRCS::Tonemap::Vignette;
        m_tonemapChromaticAberrationHDR.cs.tonemap = shaders::TonemapHDRCS::Tonemap::ChromaticAberration;
        m_tonemapVignetteChromaticAberrationHDR.cs.tonemap = shaders::TonemapHDRCS::Tonemap::VignetteChromaticAberration;
    }

    void Tonemapper::tonemap(
        CommandList& cmd,
        BufferSRV exposure,
        TextureSRV bloom,
        TextureSRV colorInput,
        TextureUAV color,
        TextureUAV luma,
        float bloomStrength,
        VignetteSettings vignette,
        ChromaticAberrationSettings chromaticAberration)
    {
        if (vignette.enabled && chromaticAberration.enabled)
        {
            CPU_MARKER(cmd.api(), "Tonemap, Vignette, Chromatic aberration");
            GPU_MARKER(cmd, "Tonemap, Vignette, Chromatic aberration");

            m_tonemapVignetteChromaticAberrationHDR.cs.Exposure = exposure;
            m_tonemapVignetteChromaticAberrationHDR.cs.Bloom = bloom;

            m_tonemapVignetteChromaticAberrationHDR.cs.ColorInput = colorInput;
            m_tonemapVignetteChromaticAberrationHDR.cs.ColorRW = color;
            m_tonemapVignetteChromaticAberrationHDR.cs.OutLuma = luma;
            m_tonemapVignetteChromaticAberrationHDR.cs.g_RcpBufferDim = { 1.0f / (float)color.width(), 1.0f / (float)color.height() };
            m_tonemapVignetteChromaticAberrationHDR.cs.g_BufferDim = { (float)color.width(), (float)color.height() };
            m_tonemapVignetteChromaticAberrationHDR.cs.g_BloomStrength = bloomStrength;

            m_tonemapVignetteChromaticAberrationHDR.cs.g_chromaticAberrationStrength = chromaticAberration.strength;

            m_tonemapVignetteChromaticAberrationHDR.cs.g_vignetteInnerRadius = vignette.innerRadius;
            m_tonemapVignetteChromaticAberrationHDR.cs.g_vignetteOuterRadius = vignette.outerRadius;
            m_tonemapVignetteChromaticAberrationHDR.cs.g_vignetteOpacity = vignette.opacity;

            cmd.bindPipe(m_tonemapVignetteChromaticAberrationHDR);
            cmd.dispatch(
                roundUpToMultiple(color.width(), 8u) / 8u,
                roundUpToMultiple(color.height(), 8u) / 8u,
                1u);
        }
        else if (vignette.enabled)
        {
            CPU_MARKER(cmd.api(), "Tonemap, Vignette");
            GPU_MARKER(cmd, "Tonemap, Vignette");

            m_tonemapVignetteHDR.cs.Exposure = exposure;
            m_tonemapVignetteHDR.cs.Bloom = bloom;

            m_tonemapVignetteHDR.cs.ColorRW = color;
            m_tonemapVignetteHDR.cs.OutLuma = luma;
            m_tonemapVignetteHDR.cs.g_RcpBufferDim = { 1.0f / (float)color.width(), 1.0f / (float)color.height() };
            m_tonemapVignetteHDR.cs.g_BufferDim = { (float)color.width(), (float)color.height() };
            m_tonemapVignetteHDR.cs.g_BloomStrength = bloomStrength;

            m_tonemapVignetteHDR.cs.g_vignetteInnerRadius = vignette.innerRadius;
            m_tonemapVignetteHDR.cs.g_vignetteOuterRadius = vignette.outerRadius;
            m_tonemapVignetteHDR.cs.g_vignetteOpacity = vignette.opacity;

            cmd.bindPipe(m_tonemapVignetteHDR);
            cmd.dispatch(
                roundUpToMultiple(color.width(), 8u) / 8u,
                roundUpToMultiple(color.height(), 8u) / 8u,
                1u);
        }
        else if (chromaticAberration.enabled)
        {
            CPU_MARKER(cmd.api(), "Tonemap, Chromatic aberration");
            GPU_MARKER(cmd, "Tonemap, Chromatic aberration");

            m_tonemapChromaticAberrationHDR.cs.Exposure = exposure;
            m_tonemapChromaticAberrationHDR.cs.Bloom = bloom;

            m_tonemapChromaticAberrationHDR.cs.ColorInput = colorInput;
            m_tonemapChromaticAberrationHDR.cs.ColorRW = color;
            m_tonemapChromaticAberrationHDR.cs.OutLuma = luma;
            m_tonemapChromaticAberrationHDR.cs.g_RcpBufferDim = { 1.0f / (float)color.width(), 1.0f / (float)color.height() };
            m_tonemapChromaticAberrationHDR.cs.g_BufferDim = { (float)color.width(), (float)color.height() };
            m_tonemapChromaticAberrationHDR.cs.g_BloomStrength = bloomStrength;
            
            m_tonemapChromaticAberrationHDR.cs.g_chromaticAberrationStrength = chromaticAberration.strength;

            cmd.bindPipe(m_tonemapChromaticAberrationHDR);
            cmd.dispatch(
                roundUpToMultiple(color.width(), 8u) / 8u,
                roundUpToMultiple(color.height(), 8u) / 8u,
                1u);
        }
        else
        {
            CPU_MARKER(cmd.api(), "Tonemap");
            GPU_MARKER(cmd, "Tonemap");

            m_tonemapDefaultHDR.cs.Exposure = exposure;
            m_tonemapDefaultHDR.cs.Bloom = bloom;

            m_tonemapDefaultHDR.cs.ColorRW = color;
            m_tonemapDefaultHDR.cs.OutLuma = luma;
            m_tonemapDefaultHDR.cs.g_RcpBufferDim = { 1.0f / (float)color.width(), 1.0f / (float)color.height() };
            m_tonemapDefaultHDR.cs.g_BufferDim = { (float)color.width(), (float)color.height() };
            m_tonemapDefaultHDR.cs.g_BloomStrength = bloomStrength;

            cmd.bindPipe(m_tonemapDefaultHDR);
            cmd.dispatch(
                roundUpToMultiple(color.width(), 8u) / 8u,
                roundUpToMultiple(color.height(), 8u) / 8u,
                1u);
        }
    }

}
