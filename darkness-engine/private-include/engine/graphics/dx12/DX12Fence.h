#pragma once

#include "engine/graphics/FenceImplIf.h"
#include "engine/graphics/dx12/DX12FenceStorage.h"
#include "engine/graphics/Fence.h"
struct ID3D12Fence;

namespace engine
{
    namespace implementation
    {
        class DeviceImplIf;
        class DeviceImplDX12;
        class FenceImplDX12 : public FenceImplIf
        {
        public:
            FenceImplDX12(const DeviceImplIf* device, const char* name);
            ~FenceImplDX12();

            FenceImplDX12(const FenceImplDX12&) = delete;
            FenceImplDX12(FenceImplDX12&&) = delete;
            FenceImplDX12& operator=(const FenceImplDX12&) = delete;
            FenceImplDX12& operator=(FenceImplDX12&&) = delete;

            ID3D12Fence* native();
            ID3D12Fence* native() const;

            void increaseCPUValue() override;
            engine::FenceValue currentCPUValue() const override;
            engine::FenceValue currentGPUValue() const override;

            void blockUntilSignaled() override;
            void blockUntilSignaled(engine::FenceValue value) override;

            bool signaled() const override;
            bool signaled(engine::FenceValue value) const override;
 
            void reset() override;

        private:
            ID3D12Fence* m_fence;
            const DeviceImplDX12* m_device;
            void* m_fenceEvent;
            FenceValue m_fenceValue;
        };
    }
}

