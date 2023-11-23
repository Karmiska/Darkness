#include "tools/LinearAllocator.h"
#include "tools/ToolsCommon.h"
#include "tools/Debug.h"

namespace tools
{
    LinearAllocator2::LinearAllocator2(tools::ByteRange range)
        : m_range{ range }
        , m_currentPosition{ 0 }
    {}

    void* LinearAllocator2::allocate(size_t bytes)
    {
        return allocate(bytes, 4);
    }

    void* LinearAllocator2::allocate(size_t bytes, size_t align)
    {
        uintptr_t allocationPosition = roundUpToMultiple(m_range.start + m_currentPosition, align);
        void* result = reinterpret_cast<void*>(allocationPosition);
        m_currentPosition = allocationPosition + bytes - m_range.start;
        ASSERT(m_currentPosition <= m_range.sizeBytes(), "Ran out of memory from LinearAllocator");
        return result;
    }

    void LinearAllocator2::free(void* /*ptr*/)
    {
        // nop
    }

    size_t LinearAllocator2::offset(void* ptr) const
    {
        return static_cast<size_t>(reinterpret_cast<uintptr_t>(ptr) - m_range.start);
    }
}
