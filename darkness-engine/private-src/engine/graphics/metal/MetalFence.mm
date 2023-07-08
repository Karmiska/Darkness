#include "engine/graphics/metal/MetalFence.h"
#include "engine/graphics/metal/MetalHeaders.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/metal/MetalDevice.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        FenceImpl::FenceImpl(const DeviceImpl& device)
        : m_device{ device }
        {
        }
        
        FenceImpl::~FenceImpl()
        {
        }
        
        MetalFence& FenceImpl::native()
        {
            return m_fence;
        }
        
        const MetalFence& FenceImpl::native() const
        {
            return m_fence;
        }
        
        void FenceImpl::reset()
        {
        }
        
        bool FenceImpl::signaled() const
        {
            return false;
        }
        
        void FenceImpl::blockUntilSignaled() const
        {
        }
    }
}
