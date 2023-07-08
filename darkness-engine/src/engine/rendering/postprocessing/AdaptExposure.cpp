#include "engine/rendering/postprocessing/AdaptExposure.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    AdaptExposure::AdaptExposure(Device& device)
        : m_adaptiveExposure{ device.createPipeline<shaders::AdaptiveExposure>() }
    {
    }

    void AdaptExposure::adapt(
        CommandList& cmd,
        BufferSRV histogram,
        BufferUAV exposure,
        float targetLuminance,
        float adaptationRate,
        float minExposure,
        float maxExposure,
        unsigned int pixelCount)
    {
        CPU_MARKER(cmd.api(), "Adaptive exposure");
        GPU_MARKER(cmd, "Adaptive exposure");

        m_adaptiveExposure.cs.Histogram = histogram;
        m_adaptiveExposure.cs.Exposure = exposure;
        m_adaptiveExposure.cs.TargetLuminance = targetLuminance;
        m_adaptiveExposure.cs.AdaptationRate = adaptationRate;
        m_adaptiveExposure.cs.MinExposure = minExposure;
        m_adaptiveExposure.cs.MaxExposure = maxExposure;
        m_adaptiveExposure.cs.PixelCount = { pixelCount };

        cmd.bindPipe(m_adaptiveExposure);
        cmd.dispatch(
            1u,
            1u,
            1u);
    }

}
