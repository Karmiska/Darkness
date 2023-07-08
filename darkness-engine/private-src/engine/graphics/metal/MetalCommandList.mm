#include "engine/graphics/metal/MetalCommandList.h"
#include "engine/graphics/metal/MetalHeaders.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/PipelineState.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/DescriptorHandle.h"
#include "engine/primitives/Color.h"
#include "engine/graphics/metal/MetalDevice.h"
#include "engine/graphics/metal/MetalCommandAllocator.h"
#include "engine/graphics/metal/MetalPipelineState.h"
#include "engine/graphics/metal/MetalDescriptorHandle.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        CommandListImpl::CommandListImpl(const Device& device)
        : m_device{ device }
        , m_finishedSemaphore{ Semaphore(device) }
        {
        }
        
        MetalCommandList& CommandListImpl::native()
        {
            return m_commandList;
        }
        
        const MetalCommandList& CommandListImpl::native() const
        {
            return m_commandList;
        }
        
        Semaphore& CommandListImpl::finishedSemaphore()
        {
            return m_finishedSemaphore;
        }
        
        const Semaphore& CommandListImpl::finishedSemaphore() const
        {
            return m_finishedSemaphore;
        }
        
        void CommandListImpl::reset(const PipelineState& /*pipelineState*/)
        {
            
        }
        
        void CommandListImpl::setResourceBarrier(MetalBarrier barrier) const
        {
            m_barrier = barrier;
        }
        
        MetalBarrier& CommandListImpl::barrier()
        {
            return m_barrier;
        }
        
        const MetalBarrier& CommandListImpl::barrier() const
        {
            return m_barrier;
        }
        
        void CommandListImpl::setRenderTargets(std::vector<DescriptorHandle> /*targets*/)
        {
            
        }
        
        void CommandListImpl::clearRenderTargetView(const DescriptorHandle& /*target*/, const Color4f& /*color*/)
        {
            
        }
        
        void CommandListImpl::beginRenderPass(const PipelineState& pipeline, int frameBufferIndex)
        {
        }
        
        void CommandListImpl::begin()
        {
            
        }
        
        void CommandListImpl::copyBuffer(
                                         const Buffer& srcBuffer,
                                         Buffer& dstBuffer,
                                         uint64_t elements,
                                         uint32_t srcStartElement,
                                         uint32_t dstStartElement)
        {
        }
        
        void CommandListImpl::bindPipeline(const PipelineState& pipeline)
        {
        }
        
        void CommandListImpl::bindVertexBuffer(const Buffer& buffer)
        {
        }
        
        void CommandListImpl::bindIndexBuffer(const Buffer& buffer)
        {
        }
        
        void CommandListImpl::bindDescriptorSets(const PipelineState& pipeline, const DescriptorHandle& descriptor)
        {
        }
        
        void CommandListImpl::clearBuffer(Buffer& buffer, uint32_t value, uint32_t startElement, uint32_t numElements)
        {
        }
        
        void CommandListImpl::clearTexture(Texture& texture, const SubResource& subResource, const Color4f& color)
        {
        }
        
        void CommandListImpl::draw()
        {
        }
        
        void CommandListImpl::drawIndexed(
                                          uint32_t indexCount,
                                          uint32_t instanceCount,
                                          uint32_t firstIndex,
                                          int32_t vertexOffset,
                                          uint32_t firstInstance)
        {
        }
        
        void CommandListImpl::endRenderPass()
        {
        }
        
        void CommandListImpl::end()
        {
        }
        
        /*void CommandListImpl::transitionTexture(const Texture& image, ImageLayout from, ImageLayout to)
        {
        }*/
        
        void CommandListImpl::copyTexture(const Texture& src, Texture& dst)
        {
        }
    }
}
