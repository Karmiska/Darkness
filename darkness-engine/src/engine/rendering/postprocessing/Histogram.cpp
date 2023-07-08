#include "engine/rendering/postprocessing/Histogram.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    Histogram::Histogram(Device& device)
        : m_generateHistogram{ device.createPipeline<shaders::GenerateHistogram>() }
        , m_debugDrawHistogram{ device.createPipeline<shaders::DebugDrawHistogram>() }
    {
        m_histogramUAV = device.createBufferUAV(BufferDescription()
            .elements(256)
            .elementSize(4)
            .format(Format::R32_UINT)
            .name("Histogram UAV"));
        m_histogramSRV = device.createBufferSRV(m_histogramUAV);
    }

    void Histogram::histogram(
        CommandList& cmd,
        TextureSRV luma)
    {
        CPU_MARKER(cmd.api(), "Generate histogram");
        GPU_MARKER(cmd, "Generate histogram");

        cmd.clearBuffer(m_histogramUAV, 0);

        m_generateHistogram.cs.Histogram = m_histogramUAV;
        m_generateHistogram.cs.LumaBuf = luma;
        m_generateHistogram.cs.width.x = static_cast<uint32_t>(luma.width());
        m_generateHistogram.cs.height.x = static_cast<uint32_t>(luma.height());

        cmd.bindPipe(m_generateHistogram);
        cmd.dispatch(
            roundUpToMultiple(luma.width(), 16ull) / 16ull,
            roundUpToMultiple(luma.height(), luma.height()) / luma.height(),
            1ull);
    }

    void Histogram::drawDebugHistogram(
        CommandList& cmd,
        BufferSRV exposure,
        TextureUAV color)
    {
        CPU_MARKER(cmd.api(), "Debug histogram");
        GPU_MARKER(cmd, "Debug histogram");

        m_debugDrawHistogram.cs.Histogram = m_histogramSRV;
        m_debugDrawHistogram.cs.Exposure = exposure;
        m_debugDrawHistogram.cs.ColorBuffer = color;

        cmd.bindPipe(m_debugDrawHistogram);
        cmd.dispatch(
            1ull,
            32ull,
            1ull);
    }

    BufferUAV Histogram::histogramUAV()
    {
        return m_histogramUAV;
    }

    BufferSRV Histogram::histogramSRV()
    {
        return m_histogramSRV;
    }

}
