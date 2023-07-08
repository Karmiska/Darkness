#pragma once

#include "engine/graphics/SamplerImplIf.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Common.h"
#include "containers/memory.h"

namespace engine
{
    struct SamplerDescription;
    class Device;

    namespace implementation
    {
        class SamplerImplDX12 : public SamplerImplIf
        {
        public:
            SamplerImplDX12(
                const Device& device,
                const SamplerDescription& desc);
            
            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            uint64_t uniqueId() const
            {
                return m_descriptorHandle.uniqueId();
            }
        private:
            DescriptorHandleDX12 m_descriptorHandle;
        };
    }
}

