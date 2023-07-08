#pragma once

#include "tools/ByteRange.h"

namespace tools
{
    class OffsetAllocatorInterface
    {
    public:
        virtual ~OffsetAllocatorInterface() {};
        virtual void* allocate(size_t bytes) = 0;
        virtual void* allocate(size_t bytes, size_t align) = 0;
        virtual void free(void* ptr) = 0;
        virtual size_t offset(void* ptr) const = 0;
        virtual void* ptrFromOffset(size_t offset) const = 0;
    };
}
