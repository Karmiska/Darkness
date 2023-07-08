#pragma once

#include <cstdint>

namespace engine
{
    enum class RootSignatureFlags
    {
        None,
        AllowInputAssemblerInputLayout,
        DenyVertexShaderRootAccess,
        DenyHullShaderRootAccess,
        DenyDomainShaderRootAccess,
        DenyGeometryShaderRootAccess,
        DenyPixelShaderRootAccess,
        AllowStreamOutput
    };
    class RootParameter;
    struct SamplerDescription;
    using ShaderVisibility = std::uint32_t;

    namespace implementation
    {
        class RootSignatureImplIf
        {
        public:
            virtual ~RootSignatureImplIf() {};
            virtual void reset(int rootParameterCount, int staticSamplerCount) = 0;
            virtual void initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility) = 0;
            virtual void finalize(RootSignatureFlags flags = RootSignatureFlags::None) = 0;
            virtual void enableNullDescriptors(bool texture, bool writeable) = 0;
            virtual size_t rootParameterCount() const = 0;
            virtual RootParameter& operator[](size_t index) = 0;
            virtual const RootParameter& operator[](size_t index) const = 0;
        };
    }
}