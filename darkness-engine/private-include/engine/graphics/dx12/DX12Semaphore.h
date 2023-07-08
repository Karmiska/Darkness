#pragma once

#include "engine/graphics/SemaphoreImplIf.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "tools/ComPtr.h"

namespace engine
{
    class Device;
    namespace implementation
    {
        class SemaphoreImplDX12 : public SemaphoreImplIf
        {
        public:
            SemaphoreImplDX12(const Device& device);

            ID3D12Fence* native();
            ID3D12Fence* native() const;

            void reset() override;
            bool signaled() const override;
        private:
            tools::ComPtr<ID3D12Fence> m_fence;
            void* m_fenceEvent;
            unsigned long long m_fenceValue;
        };
    }
}

