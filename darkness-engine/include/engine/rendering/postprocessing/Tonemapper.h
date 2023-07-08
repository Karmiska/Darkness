#pragma once

#include "engine/graphics/Device.h"
#include "shaders/core/postprocess/TonemapHDR.h"
#include "components/PostprocessComponent.h"

namespace engine
{
    class CommandList;
    class Tonemapper
    {
    public:
        Tonemapper(Device& device);

        void tonemap(
            CommandList& cmd,
            BufferSRV exposure,
            TextureSRV bloom,
            TextureSRV colorInput,
            TextureUAV color,
            TextureUAV luma,
            float bloomStrength,
            VignetteSettings vignette,
            ChromaticAberrationSettings chromaticAberration);

    private:
        engine::Pipeline<shaders::TonemapHDR> m_tonemapDefaultHDR;
        engine::Pipeline<shaders::TonemapHDR> m_tonemapVignetteHDR;
        engine::Pipeline<shaders::TonemapHDR> m_tonemapChromaticAberrationHDR;
        engine::Pipeline<shaders::TonemapHDR> m_tonemapVignetteChromaticAberrationHDR;

    };
}
