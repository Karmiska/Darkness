#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/rendering/postprocessing/Exposure.h"
#include "shaders/core/postprocess/ExtractBloomLuminance.h"
#include "shaders/core/postprocess/ExtractLuminance.h"
#include "shaders/core/postprocess/DownsampleBloom4.h"
#include "shaders/core/postprocess/UpsampleAndBlur.h"
#include "shaders/core/postprocess/BloomBlur.h"

namespace engine
{
    class CommandList;
    class TextureSRV;
    class BloomAndLuminance
    {
    public:
        BloomAndLuminance(Device& device);

        void extract(
            CommandList& cmd,
            TextureSRV src,
            Exposure& exposure,
            bool bloomEnabled,
            float threshold);

        TextureUAV lumaLRUAV();
        TextureSRV lumaLRSRV();

        //engine::vector<engine::vector<TextureUAV>>& bloomUAV();
        //engine::vector<engine::vector<TextureSRV>>& bloomSRV();

        TextureSRV bloom();

    private:
        Device& m_device;

        engine::Pipeline<shaders::ExtractBloomLuminance> m_extractBloomLuminance;
        engine::Pipeline<shaders::ExtractLuminance> m_extractLuminance;
        engine::Pipeline<shaders::DownsampleBloom4> m_downSampleBloom4;
        engine::Pipeline<shaders::UpsampleAndBlur> m_upsampleAndBlur;
        engine::Pipeline<shaders::BloomBlur> m_blur;

        void refreshInternalTextures(TextureSRV src, bool bloomEnabled);

        TextureUAVOwner m_lumaLRUAV;
        TextureSRVOwner m_lumaLRSRV;

        engine::vector<engine::vector<TextureUAVOwner>> m_bloomUAV;
        engine::vector<engine::vector<TextureSRVOwner>> m_bloomSRV;

        engine::vector<engine::vector<TextureUAV>> m_bloomUAVHandles;
        engine::vector<engine::vector<TextureSRV>> m_bloomSRVHandles;

    };
}
