#pragma once

#include "engine/graphics/RootSignatureImplIf.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/RootSignature.h"
#include "containers/memory.h"

struct ID3D12RootSignature;

namespace engine
{
    class Device;
    namespace implementation
    {
        class RootSignatureImplDX12 : public RootSignatureImplIf
        {
        public:
            RootSignatureImplDX12(const Device& device, int rootParameterCount = 0, int staticSamplerCount = 0);
            ~RootSignatureImplDX12();

            void reset(int rootParameterCount, int staticSamplerCount);
            void initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility);
            void finalize(RootSignatureFlags flags = RootSignatureFlags::None);
            void enableNullDescriptors(bool texture, bool writeable) override;
            size_t rootParameterCount() const;
            RootParameter& operator[](size_t index);
            const RootParameter& operator[](size_t index) const;

            RootSignatureImplDX12(const RootSignatureImplDX12&) = delete;
            RootSignatureImplDX12(RootSignatureImplDX12&&) = delete;
            RootSignatureImplDX12& operator=(const RootSignatureImplDX12&) = delete;
            RootSignatureImplDX12& operator=(RootSignatureImplDX12&&) = delete;

            ID3D12RootSignature* native();
            ID3D12RootSignature* native() const;
        private:
            const Device& m_device;
            ID3D12RootSignature* m_signature;

            engine::vector<RootParameter> m_parameters;
            engine::unique_ptr<StaticSamplerDescription[]> m_samplers;

            int m_parameterCount;
            int m_samplerCount;
            size_t m_numInitializedStaticSamplers;
        };
    }
}

