#pragma once

#include "EcsShared.h"
#include "containers/BitSet.h"

namespace ecs
{
    class ChunkStorageAllocation
    {
    public:
        ChunkStorageAllocation()
            : m_allocation{ nullptr }
            , m_allocationCount{ 0 }
            , m_allocationsInUse{ 0 }
            , m_chunkSizeBytes{ 0 }
            , m_used{}
        {}

        ChunkStorageAllocation(size_t bytes, size_t chunkSizeBytes, bool clear = false)
            : m_allocation{ _aligned_malloc(bytes, ChunkAlignment) }
            , m_allocationCount{ bytes / chunkSizeBytes }
            , m_allocationsInUse{ 0 }
            , m_chunkSizeBytes{ chunkSizeBytes }
            , m_used{}
        {
            if (m_allocation && clear)
                memset(m_allocation, 0, bytes);
            m_used.resize(m_allocationCount, true);
        }

        ChunkStorageAllocation(const ChunkStorageAllocation&) = delete;
        ChunkStorageAllocation& operator=(const ChunkStorageAllocation&) = delete;

        ChunkStorageAllocation(ChunkStorageAllocation&& allocation) noexcept
            : m_allocation{ nullptr }
            , m_allocationCount{ 0 }
            , m_allocationsInUse{ 0 }
            , m_chunkSizeBytes{ 0 }
            , m_used{}
        {
            std::swap(m_allocation, allocation.m_allocation);
            std::swap(m_allocationCount, allocation.m_allocationCount);
            std::swap(m_allocationsInUse, allocation.m_allocationsInUse);
            std::swap(m_chunkSizeBytes, allocation.m_chunkSizeBytes);
            std::swap(m_used, allocation.m_used);
        }
        ChunkStorageAllocation& operator=(ChunkStorageAllocation&& allocation) noexcept
        {
            std::swap(m_allocation, allocation.m_allocation);
            std::swap(m_allocationCount, allocation.m_allocationCount);
            std::swap(m_allocationsInUse, allocation.m_allocationsInUse);
            std::swap(m_chunkSizeBytes, allocation.m_chunkSizeBytes);
            std::swap(m_used, allocation.m_used);
            return *this;
        }

        ~ChunkStorageAllocation()
        {
            if (m_allocation)
                _aligned_free(m_allocation);
        }

        void* allocate()
        {
            if (m_allocationsInUse >= m_allocationCount)
                return nullptr;

            auto next = m_used.begin();
            ASSERT(next != m_used.end(), "This is a bug. m_allocationCount claims we have free chunk but m_used didn't find any");
            m_used.clear(*next);
            ++m_allocationsInUse;
            return static_cast<uint8_t*>(m_allocation) + (m_chunkSizeBytes * *next);
        }

        void deallocate(void* ptr)
        {
            --m_allocationsInUse;
            auto chunkIndex = (reinterpret_cast<uintptr_t>(ptr) - 
                reinterpret_cast<uintptr_t>(m_allocation)) / m_chunkSizeBytes;
            m_used.set(chunkIndex);
        }

        bool full() const
        {
            return m_allocationsInUse == m_allocationCount;
        }

        bool empty() const
        {
            return m_allocationsInUse == 0;
        }
    private:
        void* m_allocation;
        size_t m_allocationCount;
        size_t m_allocationsInUse;
        size_t m_chunkSizeBytes;
        engine::BitSetDynamic m_used;
    };
}