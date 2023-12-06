#pragma once

#include "EcsShared.h"

namespace ecs
{
    class ChunkStorageAllocation
    {
    public:
        ChunkStorageAllocation()
            : m_allocation{ nullptr }
            , m_inUse{ 0 }
            , m_allocationCount{ 0 }
        {}

        ChunkStorageAllocation(size_t bytes, bool clear = false)
            : m_allocation{ _aligned_malloc(bytes, ChunkDataAlignment) }
            , m_inUse{ 0 }
            , m_allocationCount{ 0 }
        {
            if (m_allocation && clear)
                memset(m_allocation, 0, bytes);
        }

        ChunkStorageAllocation(const ChunkStorageAllocation&) = delete;
        ChunkStorageAllocation& operator=(const ChunkStorageAllocation&) = delete;

        ChunkStorageAllocation(ChunkStorageAllocation&& allocation) noexcept
            : m_allocation{ nullptr }
            , m_inUse{ 0 }
            , m_allocationCount{ 0 }
        {
            std::swap(m_allocation, allocation.m_allocation);
            std::swap(m_inUse, allocation.m_inUse);
            std::swap(m_allocationCount, allocation.m_allocationCount);
        }
        ChunkStorageAllocation& operator=(ChunkStorageAllocation&& allocation) noexcept
        {
            std::swap(m_allocation, allocation.m_allocation);
            std::swap(m_inUse, allocation.m_inUse);
            std::swap(m_allocationCount, allocation.m_allocationCount);
            return *this;
        }

        ~ChunkStorageAllocation()
        {
            if (m_allocation)
                _aligned_free(m_allocation);
        }

        void* allocate(size_t bytes)
        {
            uintptr_t alignedPtr = roundUpToMultiple(m_inUse, ChunkDataAlignment);
            auto newInUse = m_inUse + ((alignedPtr - m_inUse) + bytes);
            if (newInUse > ChunkStorageAllocationSize)
                return nullptr;
            ++m_allocationCount;
            m_inUse = newInUse;
            return static_cast<uint8_t*>(m_allocation) + alignedPtr;
        }

        void allocate()
        {
            ++m_allocationCount;
        }

        void deallocate()
        {
            --m_allocationCount;
        }

        size_t allocationCount() const { return m_allocationCount; }
    private:
        void* m_allocation;
        uintptr_t m_inUse;
        size_t m_allocationCount;
    };
}