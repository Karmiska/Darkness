#pragma once

#include "tools/Allocator.h"
#include "tools/ByteRange.h"

namespace tools
{
    class LinearAllocator : public OffsetAllocatorInterface
    {
    public:
        LinearAllocator() = default;
        LinearAllocator(ByteRange range);

        void* allocate(size_t bytes) override;
        void* allocate(size_t bytes, size_t align) override;
        void free(void* ptr) override;
        size_t offset(void* ptr) const override;
    private:
        ByteRange m_range;
        uintptr_t m_currentPosition;
    };
}
