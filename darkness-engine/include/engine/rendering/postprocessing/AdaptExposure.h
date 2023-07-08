#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/ResourceOwners.h"
#include "shaders/core/postprocess/AdaptiveExposure.h"

namespace engine
{
    class CommandList;
    class AdaptExposure
    {
    public:
        AdaptExposure(Device& device);

        void adapt(
            CommandList& cmd,
            BufferSRV histogram,
            BufferUAV exposure,
            float targetLuminance,
            float adaptationRate,
            float minExposure,
            float maxExposure,
            unsigned int pixelCount);
    private:
        engine::Pipeline<shaders::AdaptiveExposure> m_adaptiveExposure;
    };
}
