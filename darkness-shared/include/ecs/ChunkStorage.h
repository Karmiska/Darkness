#pragma once

#include "Chunk.h"
#include "tools/ToolsCommon.h"
#include <atomic>

namespace ecs
{
    // when allocating new chunks, zeroing the memory is a slow operation.
    // one can avoid that by allocating large enough chunk storage allocation,
    // because when ChunkStorage is created, it creates a bunch of allocations in it's constructor
    // which is called by Ecs constructor. (so all memory can be allocated beforehand if need be)
    constexpr bool ZeroChunkMemory = false;
    constexpr size_t ChunkStorageAllocationSize = 16ull * 1024ull * 1024ull;
    constexpr size_t PreallocatedChunkStorageSizeBytes = 0;// 5ull * 1024ull * 1024ull * 1024ull;

    class ChunkStorage
    {
    public:
        ChunkStorage(
            TypeStorage& componentTypeStorage,
            ArcheTypeStorage& archeTypeStorage)
            : m_componentTypeStorage{ componentTypeStorage }
            , m_archeTypeStorage{ archeTypeStorage }
            , m_currentAllocationIndex{ 0 }
        {
            size_t bytesAllocated = 0;
            while (bytesAllocated < PreallocatedChunkStorageSizeBytes)
            {
                m_storageAllocations.emplace_back(ChunkStorageAllocation{ ChunkStorageAllocationSize, ZeroChunkMemory });
                bytesAllocated += ChunkStorageAllocationSize;
            }
            if (m_storageAllocations.size() == 0)
                m_storageAllocations.emplace_back(ChunkStorageAllocation{ ChunkStorageAllocationSize, ZeroChunkMemory });
        }

        Chunk* allocateChunk(ComponentArcheTypeId archeType)
        {
            if (archeType >= m_freeChunks.size())
                m_freeChunks.resize(archeType + 1);

            auto& chunkList = m_freeChunks[archeType];
            if (chunkList.size())
            {
                auto res = chunkList.top();
                chunkList.pop();
                return res;
            }

            return allocateNewChunk(archeType);
        }

        void freeChunk(ComponentArcheTypeId archeType, Chunk* chunk)
        {
            m_freeChunks[archeType].push(chunk);
        }

    private:
        TypeStorage& m_componentTypeStorage;
        ArcheTypeStorage& m_archeTypeStorage;

        class ChunkStorageAllocation
        {
        public:
            ChunkStorageAllocation()
                : m_allocation{ nullptr }
            {}

            ChunkStorageAllocation(size_t bytes, bool clear = false)
                : m_allocation{ _aligned_malloc(bytes, ChunkDataAlignment) }
                , m_inUse{ 0 }
            {
                if(m_allocation && clear)
                    memset(m_allocation, 0, bytes);
            }

            ChunkStorageAllocation(const ChunkStorageAllocation&) = delete;
            ChunkStorageAllocation& operator=(const ChunkStorageAllocation&) = delete;

            ChunkStorageAllocation(ChunkStorageAllocation&& allocation) noexcept
                : m_allocation{ nullptr }
            {
                std::swap(m_allocation, allocation.m_allocation);
                m_inUse.store(allocation.m_inUse.load());
            }
            ChunkStorageAllocation& operator=(ChunkStorageAllocation&& allocation) noexcept
            {
                std::swap(m_allocation, allocation.m_allocation);
                m_inUse.store(allocation.m_inUse.load());
                return *this;
            }
            
            ~ChunkStorageAllocation()
            {
                if(m_allocation)
                    _aligned_free(m_allocation);
            }

            void* allocate(size_t bytes)
            {
                uintptr_t current = m_inUse.load(std::memory_order_relaxed);
                uintptr_t alignedPtr = roundUpToMultiple(current, ChunkDataAlignment);
                uintptr_t next = alignedPtr + bytes;
                if (next > ChunkStorageAllocationSize)
                    return nullptr;

                while (!std::atomic_compare_exchange_weak_explicit(
                    &m_inUse, &current, next,
                    std::memory_order_release, std::memory_order_relaxed))
                {
                    current = m_inUse.load(std::memory_order_relaxed);
                    alignedPtr = roundUpToMultiple(current, ChunkDataAlignment);
                    next = alignedPtr + bytes;
                    if (next > ChunkStorageAllocationSize)
                        return nullptr;
                };
                return static_cast<uint8_t*>(m_allocation) + alignedPtr;
            }
        private:
            void* m_allocation;
            std::atomic<uintptr_t> m_inUse;
        };

        void* getStorageAllocation(size_t bytes)
        {
            auto chunkAllocation = m_storageAllocations[m_currentAllocationIndex].allocate(bytes);
            if (!chunkAllocation)
            {
                ++m_currentAllocationIndex;
                if(m_currentAllocationIndex >= m_storageAllocations.size())
                    m_storageAllocations.emplace_back(ChunkStorageAllocation{ ChunkStorageAllocationSize, ZeroChunkMemory });
                
                chunkAllocation = m_storageAllocations[m_currentAllocationIndex].allocate(bytes);
            }

            ASSERT(chunkAllocation, "Failed to allocate chunk memory");
            return chunkAllocation;
        }

        Chunk* allocateNewChunk(ComponentArcheTypeId archeType)
        {
            auto chunk = new Chunk(m_archeTypeStorage, archeType);
            auto chunkBytes = (chunk->elementSizeBytes() * chunk->capacity()) + chunk->typePaddingSizeBytes();
            chunk->initialize(m_componentTypeStorage, getStorageAllocation(chunkBytes), chunkBytes);
            return chunk;
        }
    

    private:
        engine::vector<std::stack<Chunk*>> m_freeChunks;
        engine::vector<ChunkStorageAllocation> m_storageAllocations;
        size_t m_currentAllocationIndex;
        std::mutex m_mutex;
    };
}
