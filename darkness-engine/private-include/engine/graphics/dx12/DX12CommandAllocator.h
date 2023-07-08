#pragma once

#include "engine/graphics/CommandAllocatorImplIf.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/CommonNoDep.h"
#include "tools/ComPtr.h"

namespace engine
{
    namespace implementation
    {
        class DeviceImplDX12;
        class CommandAllocatorImplDX12 : public CommandAllocatorImplIf
        {
        public:
            CommandAllocatorImplDX12(const DeviceImplDX12& device, CommandListType type, const char* name);

            void reset() override;
            CommandListType type() const override;

            ID3D12CommandAllocator* native() const;
        private:
            tools::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
            CommandListType m_type;
        };
    }
}

