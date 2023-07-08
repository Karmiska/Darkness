#include "tools/SyncAllocator.h"
#include "tools/Debug.h"

namespace engine
{
    SyncAllocator::SyncAllocator(
        Device& device,
        const BufferDescription& desc)
        : m_device{ device }

        // CPU buffer
        , m_cpuBuffer{
            device.createBufferSRV(BufferDescription()
                .usage(ResourceUsage::Upload)
                .format(Format::R32G32_UINT)
                .name("SyncAllocator delta cpu")
                .elements(desc.descriptor.elements)
                .elementSize(desc.descriptor.elementSize)) }

        // GPU buffer and UAV
        , m_gpuBuffer{
            device.createBufferSRV(BufferDescription()
                .usage(ResourceUsage::GpuReadWrite)
                .format(Format::R32G32_UINT)
                .name("SyncAllocator delta cpu")
                .elements(desc.descriptor.elements)
                .elementSize(desc.descriptor.elementSize)) }
        , m_gpuBufferUAV{ device.createBufferUAV(m_gpuBuffer) }

        // buffer for change deltas
        , m_cpuDeltaSRV{ 
            device.createBufferSRV(BufferDescription()
                .usage(ResourceUsage::Upload)
                .format(Format::R32G32_UINT)
                .name("SyncAllocator delta cpu")
                .elements(desc.descriptor.elements)
                .elementSize(desc.descriptor.elementSize)) }

        // map setup
        , m_blockSizeBytes{ static_cast<uint32_t>(desc.descriptor.elementSize) }
        , m_cpuBufferPtr{ m_cpuBuffer.resource().buffer().map(m_device) }
        , m_deltaBufferPtr{ reinterpret_cast<DeltaContainer*>(m_cpuDeltaSRV.resource().buffer().map(m_device)) }

        // logical addressing
        , m_left{ 0 }
        , m_right{ 0 }
        , m_max{ static_cast<uintptr_t>(desc.descriptor.elements) }
    {
    }

    uintptr_t SyncAllocator::allocate(size_t blocks)
    {
        ASSERT(m_right + blocks <= m_max, "SyncAllocator ran out of space");
        auto res = m_right;
        m_right += blocks;
        return res;
    }

    void SyncAllocator::free(uintptr_t block, size_t count)
    {
        ASSERT(block + count <= m_max, "SyncAllocator tried to free over borders");
        if (block + count == m_max)
        {
            m_right = m_right - count;
        }
        else
        {

        }
    }

    void SyncAllocator::setDirty(uintptr_t /*block*/, size_t /*count*/)
    {
        /*for(int i = 0; i < count; ++i)
            m_deltaBufferPtr[block + i]*/
    }

    void SyncAllocator::synchronize()
    {
        handleDirty();
        handleMove();
    }

    void SyncAllocator::handleDirty()
    {
        // copy from CPU to GPU
    }

    void SyncAllocator::handleMove()
    {
        // move
    }
}
