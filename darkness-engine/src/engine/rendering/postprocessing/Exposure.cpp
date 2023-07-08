#include "engine/rendering/postprocessing/Exposure.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    Exposure::Exposure(Device& device)
        : m_device{ device }
    {
        float Exposure = 2.0f;
        const float kInitialMinLog = -12.0f;
        const float kInitialMaxLog = 4.0f;

        float initExposure[] =
        {
            Exposure, 1.0f / Exposure, Exposure, 0.0f,
            kInitialMinLog, kInitialMaxLog, kInitialMaxLog - kInitialMinLog, 1.0f / (kInitialMaxLog - kInitialMinLog)
        };
        tools::ByteRange range(&initExposure, &initExposure + (sizeof(float) * 8));

        m_exposureUAV = m_device.createBufferUAV(BufferDescription()
            .elementSize(sizeof(float))
            .elements(8)
            .format(Format::UNKNOWN)
            .name("Exposure values")
            .structured(true)
            .setInitialData(BufferDescription::InitialData(range, 4)));
        m_exposureSRV = m_device.createBufferSRV(m_exposureUAV);
    }

    BufferSRV Exposure::exposureSRV()
    {
        return m_exposureSRV;
    }

    BufferUAV Exposure::exposureUAV()
    {
        return m_exposureUAV;
    }
}
