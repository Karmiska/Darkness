#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "shaders/core/tools/GaussianBlur.h"

namespace engine
{
    class CommandList;
    class GaussianBlurTool
    {
    public:
        GaussianBlurTool(Device& device);
        void blur(CommandList& cmd, TextureSRV src, TextureSRV target);
        void blur(CommandList& cmd, TextureSRV src, TextureUAV target);
    private:
        Device& m_device;
        engine::Pipeline<shaders::GaussianBlur> m_gaussianBlurPipeline;
        struct BlurContainer
        {
            size_t width;
            size_t height;
            TextureRTVOwner m_blurTargetRTV[2];
            TextureSRVOwner m_blurTargetSRV[2];
            TextureUAVOwner m_blurTargetUAV[2];
        };
        engine::vector<BlurContainer> m_blurTextures;
        int refreshInternalTextures(TextureSRV src);
    };
}
