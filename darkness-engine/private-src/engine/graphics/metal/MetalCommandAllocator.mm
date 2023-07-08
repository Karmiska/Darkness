#include "engine/graphics/metal/MetalCommandAllocator.h"
#include "engine/graphics/metal/MetalHeaders.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/metal/MetalDevice.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        CommandAllocatorImpl::CommandAllocatorImpl(const Device& device)
        {
            /*ASSERT(SUCCEEDED(
                DeviceImplGet::impl(device).device()->CreateCommandAllocator(
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    __uuidof(ID3D12CommandAllocator),
                    (void**)&m_commandAllocator)));*/
        }

        CommandAllocatorImpl::~CommandAllocatorImpl()
        {
            /*if (m_commandAllocator)
            {
                m_commandAllocator->Release();
                m_commandAllocator = NULL;
            }*/
        }

        void* CommandAllocatorImpl::native() const
        {
            return m_commandAllocator;
        }

        void CommandAllocatorImpl::reset()
        {
            //ASSERT(SUCCEEDED(m_commandAllocator->Reset()));
        }
    }
}
