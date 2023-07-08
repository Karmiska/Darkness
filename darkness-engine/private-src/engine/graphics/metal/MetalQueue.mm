#include "engine/graphics/metal/MetalQueue.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/metal/MetalCommandList.h"
#include "engine/graphics/metal/MetalFence.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/metal/MetalDevice.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        QueueImpl::QueueImpl(const Device& device, CommandListType type)
        : m_queue{ }
        , m_device{ device }
        , m_needRefresh{ false }
        {
        }
        
        QueueImpl::~QueueImpl()
        {
        }
        
        const MetalQueue& QueueImpl::native() const
        {
            return m_queue;
        }
        
        void QueueImpl::submit(std::vector<CommandList>& commandLists)
        {
        }
        
        void QueueImpl::submit(const CommandList& commandList)
        {
        }
        
        void QueueImpl::submit(std::vector<CommandList>& commandLists, Fence& fence)
        {
        }
        
        void QueueImpl::submit(const CommandList& commandList, Fence& fence)
        {
        }
        
        void QueueImpl::waitForIdle() const
        {
        }
        
        void QueueImpl::signal(const Semaphore& /*semaphore*/)
        {
        }
        
        void QueueImpl::present(
                                const Semaphore& signalSemaphore,
                                const SwapChain& swapChain,
                                unsigned int chainIndex)
        {
        }
        
        bool QueueImpl::needRefresh() const
        {
            return m_needRefresh;
        }
    }
}
