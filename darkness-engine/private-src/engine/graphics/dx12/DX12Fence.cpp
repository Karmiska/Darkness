#include "engine/graphics/dx12/DX12Fence.h"
#include "engine/graphics/dx12/DX12Headers.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/dx12/DX12Device.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        FenceImplDX12::FenceImplDX12(const DeviceImplIf* device, const char* name)
            : m_fence{ nullptr }
            , m_device{ static_cast<const DeviceImplDX12*>(device) }
            , m_fenceValue{ 0 }
        {
            m_fence = m_device->fenceStorage().acquireFence(m_device->device());
            ASSERT(m_fence != nullptr, "Failed to get fence");

            m_fenceValue = m_fence->GetCompletedValue();
            m_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
            ASSERT(m_fenceEvent);


#ifndef RELEASE            
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

            m_fence->SetName(resourceName);
#endif

        }

        FenceImplDX12::~FenceImplDX12()
        {
            auto res = CloseHandle(m_fenceEvent);
            ASSERT(res == TRUE);

            if (m_fence)
            {
                m_device->fenceStorage().releaseFence(m_fence);
            }
        }

        ID3D12Fence* FenceImplDX12::native()
        {
            return m_fence;
        }

        ID3D12Fence* FenceImplDX12::native() const
        {
            return m_fence;
        }

        void FenceImplDX12::increaseCPUValue()
        {
            m_fenceValue++;
        }

        FenceValue FenceImplDX12::currentCPUValue() const
        {
            return m_fenceValue;
        }

        FenceValue FenceImplDX12::currentGPUValue() const
        {
            return m_fence->GetCompletedValue();
        }

        void FenceImplDX12::blockUntilSignaled()
        {
            // advance one
            auto currentValue = m_fence->GetCompletedValue();
            while (currentValue < m_fenceValue)
            {
                ASSERT(SUCCEEDED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)));
                WaitForSingleObject(m_fenceEvent, INFINITE);
                currentValue = m_fence->GetCompletedValue();
            }
        }

        void FenceImplDX12::blockUntilSignaled(engine::FenceValue value)
        {
            auto currentValue = m_fence->GetCompletedValue();
            while (currentValue < value)
            {
                ASSERT(SUCCEEDED(m_fence->SetEventOnCompletion(value, m_fenceEvent)));
                WaitForSingleObject(m_fenceEvent, INFINITE);
                currentValue = m_fence->GetCompletedValue();
            }
        }

        void FenceImplDX12::reset()
        {
            m_fenceValue = m_fence->GetCompletedValue();
        }

        bool FenceImplDX12::signaled() const
        {
            return m_fence->GetCompletedValue() >= m_fenceValue;
        }

        bool FenceImplDX12::signaled(FenceValue value) const
        {
            return m_fence->GetCompletedValue() >= value;
        }
    }
}
