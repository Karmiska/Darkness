#pragma once

#include "engine/graphics/RootSignatureImplIf.h"
#include "engine/graphics/CommonNoDep.h"
#include "containers/memory.h"

namespace engine
{
    struct SamplerDescription;
    class RootParameter;
    class Device;
    enum class RootSignatureFlags;

    namespace implementation
    {
        class PipelineImpl;
    }

    class RootSignature
    {
    public:
        void reset(int rootParameterCount, int staticSamplerCount);
        void initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility);
        void finalize(RootSignatureFlags flags = RootSignatureFlags::None);
        void enableNullDescriptors(bool texture, bool writeable);
        size_t rootParameterCount() const;
        RootParameter& operator[](size_t index);
        const RootParameter& operator[](size_t index) const;

        implementation::RootSignatureImplIf* native() { return m_impl.get(); }
        const implementation::RootSignatureImplIf* native() const { return m_impl.get(); }
    private:
        friend class Device;
        RootSignature(const Device& device, GraphicsApi api, int rootParameterCount = 0, int staticSamplerCount = 0);

        engine::unique_ptr<implementation::RootSignatureImplIf> m_impl;
    };
}
