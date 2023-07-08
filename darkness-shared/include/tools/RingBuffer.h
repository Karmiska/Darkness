#pragma once

#include "tools/Allocator.h"
#include "tools/ByteRange.h"

namespace tools
{
    extern void* InvalidRingBufferAllocation;
    class RingBuffer
    {
    public:
        RingBuffer() = default;
        RingBuffer(ByteRange range, size_t align = 4);

        struct AllocStruct
        {
            void* ptr;
            size_t size;
        };

        AllocStruct allocate(size_t bytes);
        AllocStruct allocate(size_t bytes, size_t align);
        void free(AllocStruct ptr);
        size_t offset(void* ptr) const;
        void* ptrFromOffset(size_t offset) const;
        size_t maxAllocationSpace() const;
		void reset();
    private:
        size_t m_alignment;
        ByteRange m_range;

		uintptr_t m_writePtr;
		uintptr_t m_lengthPtr;
        uintptr_t m_endSkip;

        engine::vector<AllocStruct> m_frees;
    };
}
