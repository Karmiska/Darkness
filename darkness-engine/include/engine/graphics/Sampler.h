#pragma once

#include "engine/graphics/SamplerImplIf.h"
#include "containers/memory.h"

namespace engine
{
    struct SamplerDescription;
    class Device;
    enum class GraphicsApi;

    class Sampler
    {
    public:
        Sampler();
        bool valid() const;

        implementation::SamplerImplIf* native() { return m_impl.get(); }
        const implementation::SamplerImplIf* native() const { return m_impl.get(); }
    private:
        friend class Device;
        Sampler(const Device& device, const SamplerDescription& desc, GraphicsApi api);

        engine::shared_ptr<implementation::SamplerImplIf> m_impl;
    };
}
