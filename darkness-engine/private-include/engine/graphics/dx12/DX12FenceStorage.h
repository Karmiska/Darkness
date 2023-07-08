#pragma once

#include <queue>

struct ID3D12Fence;
struct ID3D12Device;

namespace engine
{
    namespace implementation
    {
        class FenceStorageDX12
        {
        public:
            FenceStorageDX12() {};
            ~FenceStorageDX12();

            ID3D12Fence* acquireFence(ID3D12Device* device) const;
            void releaseFence(ID3D12Fence* fence) const;

        private:
            mutable std::queue<ID3D12Fence*> m_fences;
        };
    }
}