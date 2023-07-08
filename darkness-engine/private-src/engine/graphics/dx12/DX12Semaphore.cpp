#include "engine/graphics/dx12/DX12Semaphore.h"
#include "engine/graphics/dx12/DX12Device.h"

#include "engine/graphics/Device.h"
#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        SemaphoreImplDX12::SemaphoreImplDX12(const Device& device)
            : m_fenceValue{ 0 }
        {
            auto createRes = static_cast<const DeviceImplDX12*>(device.native())->device()->CreateFence(
                0,
                D3D12_FENCE_FLAG_NONE,
                DARKNESS_IID_PPV_ARGS(m_fence.GetAddressOf()));
            ASSERT(SUCCEEDED(createRes));

            m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
            ASSERT(m_fenceEvent);
        }

        ID3D12Fence* SemaphoreImplDX12::native()
        {
            return m_fence.Get();
        }

        ID3D12Fence* SemaphoreImplDX12::native() const
        {
            return m_fence.Get();
        }

        void SemaphoreImplDX12::reset()
        {
        }

        bool SemaphoreImplDX12::signaled() const
        {
            return m_fence->GetCompletedValue() < m_fenceValue;
        }

    }
}
