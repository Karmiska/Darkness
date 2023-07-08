#include "engine/graphics/dx12/DX12CommandAllocator.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Conversions.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/dx12/DX12Device.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        CommandAllocatorImplDX12::CommandAllocatorImplDX12(const DeviceImplDX12& device, CommandListType type, const char* name)
            : m_type{ type }
        {
            auto res = device.device()->CreateCommandAllocator(
                dxCommandListType(type),
                DARKNESS_IID_PPV_ARGS(m_commandAllocator.GetAddressOf()));
            ASSERT(SUCCEEDED(res));

#ifdef _UNICODE
            static WCHAR resourceName[1024] = {};
            size_t numCharacters;
            if (name)
                mbstowcs_s(&numCharacters, resourceName, name, 1024);
#else
            wchar_t resourceName[1024] = {};
            size_t numCharacters;
            mbstowcs_s(&numCharacters, resourceName, name, 1024);
#endif
            m_commandAllocator->SetName(resourceName);
        }

        ID3D12CommandAllocator* CommandAllocatorImplDX12::native() const
        {
            return m_commandAllocator.Get();
        }

        void CommandAllocatorImplDX12::reset()
        {
            auto res = m_commandAllocator->Reset();
            ASSERT(SUCCEEDED(res));
        }

        CommandListType CommandAllocatorImplDX12::type() const
        {
            return m_type;
        }
    }
}
