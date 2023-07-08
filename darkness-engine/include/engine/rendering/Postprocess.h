#pragma once

#include "engine/graphics/CommandList.h"
#include "engine/graphics/ResourceOwners.h"
#include "components/PostprocessComponent.h"
#include "engine/rendering/postprocessing/Exposure.h"
#include "engine/rendering/postprocessing/BloomAndLuminance.h"
#include "engine/rendering/postprocessing/Tonemapper.h"
#include "engine/rendering/postprocessing/Histogram.h"
#include "engine/rendering/postprocessing/AdaptExposure.h"

namespace engine
{
    class Postprocess
    {
    public:
        Postprocess(Device& device);
        void render(
            TextureRTV currentRenderTarget,
            TextureSRV frame,
            CommandList& cmd,
            PostprocessComponent& postprocessComponent,
            bool histogramDebug
        );

    private:
        Device& m_device;

        Exposure m_exposure;
        BloomAndLuminance m_bloomAndLuminance;
        Tonemapper m_tonemap;
        Histogram m_histogram;
        AdaptExposure m_adaptiveExposure;

        TextureUAVOwner m_colorInputUAV;
        TextureSRVOwner m_colorInputSRV;

        TextureUAVOwner m_colorUAV;
        TextureSRVOwner m_colorSRV;

        TextureUAVOwner g_LumaBufferUAV;
        TextureSRVOwner g_LumaBufferSRV;

        void refreshInternalTextures(TextureSRV src);
    };
}
