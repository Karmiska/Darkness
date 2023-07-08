#include "engine/graphics/dx12/DX12FenceStorage.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12CommonNoDep.h"
#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        ID3D12Fence* FenceStorageDX12::acquireFence(ID3D12Device* device) const
        {
            if (!m_fences.empty())
            {
                auto res = m_fences.front();
                m_fences.pop();
                return res;
            }
            else
            {
                ID3D12Fence* fence;
                auto fenceres = device->CreateFence(
                    0,
                    D3D12_FENCE_FLAG_NONE,
                    DARKNESS_IID_PPV_ARGS(&fence));
                ASSERT(SUCCEEDED(fenceres));
                return fence;
            }
        };

        void FenceStorageDX12::releaseFence(ID3D12Fence* fence) const
        {
            m_fences.push(fence);
        };

        FenceStorageDX12::~FenceStorageDX12()
        {
            while (!m_fences.empty())
            {
                m_fences.front()->Release();
                m_fences.pop();
            }
        }
    }
}