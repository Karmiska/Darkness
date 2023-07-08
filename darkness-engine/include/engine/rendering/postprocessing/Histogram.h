#pragma once

#include "engine/graphics/Device.h"
#include "shaders/core/postprocess/GenerateHistogram.h"
#include "shaders/core/postprocess/DebugDrawHistogram.h"

namespace engine
{
    class CommandList;
    class Histogram
    {
    public:
        Histogram(Device& device);

        void histogram(
            CommandList& cmd,
            TextureSRV luma);

        void drawDebugHistogram(
            CommandList& cmd,
            BufferSRV exposure,
            TextureUAV color);

        BufferUAV histogramUAV();
        BufferSRV histogramSRV();

    private:
        engine::Pipeline<shaders::GenerateHistogram> m_generateHistogram;
        engine::Pipeline<shaders::DebugDrawHistogram> m_debugDrawHistogram;

        BufferUAVOwner m_histogramUAV;
        BufferSRVOwner m_histogramSRV;
    };
}
