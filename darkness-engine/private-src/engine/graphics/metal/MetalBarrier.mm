#include "engine/graphics/metal/MetalBarrier.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Buffer.h"
#include "engine/graphics/metal/MetalCommandList.h"
#include "engine/graphics/metal/MetalBuffer.h"
#include "engine/graphics/metal/MetalConversions.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        BarrierImpl::BarrierImpl(
                                 const CommandList& commandList,
                                 ResourceBarrierFlags /*flags*/,
                                 const TextureRTV& /*resource*/,
                                 ResourceState /*before*/,
                                 ResourceState /*after*/,
                                 unsigned int /*subResource*/,
                                 const Semaphore& waitSemaphore,
                                 const Semaphore& signalSemaphore)
        //: m_barrier{ new D3D12_RESOURCE_BARRIER() }
        //, m_commandList{ commandList }
        : m_commandList{ commandList }
        {
        }
        
        void BarrierImpl::update(
                                 ResourceState /*before*/,
                                 ResourceState /*after*/)
        {
        }
        
        BarrierImpl::~BarrierImpl()
        {
        }
    }
}
