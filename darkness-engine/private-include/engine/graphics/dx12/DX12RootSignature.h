#pragma once

#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/RootSignature.h"
#include <memory>

struct ID3D12RootSignature;

namespace engine
{
    class Device;
    namespace implementation
    {
        class RootSignatureImpl
        {
        public:
            RootSignatureImpl(const Device& device, int rootParameterCount = 0, int staticSamplerCount = 0);
            ~RootSignatureImpl();

            void reset(int rootParameterCount, int staticSamplerCount);
            void initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility);
            void finalize(RootSignatureFlags flags = RootSignatureFlags::None);
            size_t rootParameterCount() const;
            RootParameter& operator[](int index);
            const RootParameter& operator[](int index) const;

            RootSignatureImpl(const RootSignatureImpl&) = delete;
            RootSignatureImpl(RootSignatureImpl&&) = delete;
            RootSignatureImpl& operator=(const RootSignatureImpl&) = delete;
            RootSignatureImpl& operator=(RootSignatureImpl&&) = delete;

            ID3D12RootSignature* native();
            ID3D12RootSignature* native() const;
        private:
            const Device& m_device;
            ID3D12RootSignature* m_signature;

            std::unique_ptr<RootParameter[]> m_parameters;
            std::unique_ptr<StaticSamplerDescription[]> m_samplers;

            int m_parameterCount;
            int m_samplerCount;
            size_t m_numInitializedStaticSamplers;

            uint32_t m_descriptorTableBitMap;
            uint32_t m_descriptorTableSize[16];
            unsigned int m_maxDescriptorCacheHandleCount;
        };
    }
}

