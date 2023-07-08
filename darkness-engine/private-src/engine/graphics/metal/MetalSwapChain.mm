#include "engine/graphics/Queue.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Buffer.h"
#include "engine/graphics/Semaphore.h"

#include "engine/graphics/metal/MetalQueue.h"
#include "engine/graphics/metal/MetalDevice.h"
#include "engine/graphics/metal/MetalBuffer.h"

#include "engine/graphics/metal/MetalSwapChain.h"

#include "platform/window/Window.h"
#include "platform/window/OsxWindow.h"

#include <stdlib.h>
#include "engine/graphics/metal/MetalHeaders.h"

#include "tools/Debug.h"

using namespace platform;

namespace engine
{
    namespace implementation
    {
        SwapChainImpl::SwapChainImpl(
                                     const Device& device,
                                     Queue& /*queue*/,
                                     bool /*fullscreen*/,
                                     bool vsync,
                                     SwapChain* oldSwapChain)
        : m_device{ device }
        , m_vsync{ vsync }
        , m_needRefresh{ false }
        {
        }
        
        SwapChainImpl::~SwapChainImpl()
        {
        }
        
        std::pair<int, int> SwapChainImpl::size() const
        {
            return{ 0, 0 };
        }
        
        uint32_t SwapChainImpl::bufferCount() const
        {
            return 0;
        }
        
        TextureRTV SwapChainImpl::renderTarget(int index)
        {
            ASSERT(static_cast<size_t>(index) < m_views.size());
            return m_views[static_cast<size_t>(index)];
        }
        
        size_t SwapChainImpl::chainLength() const
        {
            return m_views.size();
        }
        
        unsigned int SwapChainImpl::currentBackBufferIndex(const Semaphore& semaphore) const
        {
            return 0;
        }
        
        unsigned int SwapChainImpl::currentBackBufferIndex(const Fence& fence) const
        {
            return 0;
        }
        
        void SwapChainImpl::present()
        {
        }
        
        MetalSwapChain& SwapChainImpl::native()
        {
            return m_swapChain;
        }
        
        const MetalSwapChain& SwapChainImpl::native() const
        {
            return m_swapChain;
        }
        
        bool SwapChainImpl::needRefresh() const
        {
            return false;
        }
    }
}
