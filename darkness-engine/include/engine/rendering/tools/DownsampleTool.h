#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "shaders/core/tools/Downsample.h"

namespace engine
{
    class CommandList;
    class DownsampleTool
    {
    public:
        DownsampleTool(Device& device);
        void downsample(CommandList& cmd, TextureSRV src, TextureRTV dst);
    private:
        Device & m_device;
        engine::Pipeline<shaders::Downsample> m_downsamplePipeline;
    };
}
