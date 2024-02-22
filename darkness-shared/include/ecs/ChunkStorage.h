#pragma once

#include "Chunk.h"
#include "ChunkStorageAllocation.h"
#include "tools/ToolsCommon.h"
#include "containers/BitSet.h"
#include <atomic>

namespace ecs
{
    template<auto N>
    class Allocator
    {
    };

    const size_t InvalidAllocatorIndex = std::numeric_limits<size_t>::max();

    template<>
    class Allocator<4096>
    {
    public:
        Allocator()
            : m_high(0xffffffffffffffff)
        {
            for (int i = 0; i < 64; ++i)
                m_low[i] = engine::BitSet<64>(0xffffffffffffffff);
        }

        size_t allocate()
        {
            auto next = m_high.begin();
            while (next != m_high.end())
            {
                auto& nlow = m_low[*next];
                auto nextLow = nlow.begin();
                if (nextLow != nlow.end())
                {
                    nlow.clear(*nextLow);
                    return ((*next) * 64) + (*nextLow);
                }
                else
                {
                    m_high.clear(*next);
                    next++;
                }
            }
            return InvalidAllocatorIndex;
        }

        void deallocate(size_t item)
        {
            auto highIndex = item / 64;
            auto lowIndex = item - (highIndex * 64);
            m_low[highIndex].set(lowIndex);
            m_high.set(highIndex);
        }
        
    private:
        engine::BitSet<64> m_high;
        engine::BitSet<64> m_low[64];
    };

    template<>
    class Allocator<262144>
    {
    public:
        const size_t InvalidAllocatorIndex = std::numeric_limits<size_t>::max();

        Allocator()
            : m_high(0xffffffffffffffff)
        {}

        size_t allocate()
        {
            auto next = m_high.begin();
            while (next != m_high.end())
            {
                auto res = m_low[*next].allocate();
                if (res != InvalidAllocatorIndex)
                {
                    return ((*next) * 4096) + res;
                }
                else
                {
                    m_high.clear(*next);
                    next++;
                }
            }
            return InvalidAllocatorIndex;
        }

        void deallocate(size_t item)
        {
            auto highIndex = item / 4096;
            auto lowIndex = item - (highIndex * 4096);
            m_low[highIndex].deallocate(lowIndex);
            m_high.set(highIndex);
        }

    private:
        engine::BitSet<64> m_high;
        Allocator<4096> m_low[64];
    };

    template<>
    class Allocator<16777216>
    {
    public:
        const size_t InvalidAllocatorIndex = std::numeric_limits<size_t>::max();

        Allocator()
            : m_high(0xffffffffffffffff)
        {}

        size_t allocate()
        {
            auto next = m_high.begin();
            while (next != m_high.end())
            {
                auto res = m_low[*next].allocate();
                if (res != InvalidAllocatorIndex)
                {
                    return ((*next) * 262144) + res;
                }
                else
                {
                    m_high.clear(*next);
                    next++;
                }
            }
            return InvalidAllocatorIndex;
        }

        void deallocate(size_t item)
        {
            auto highIndex = item / 262144;
            auto lowIndex = item - (highIndex * 262144);
            m_low[highIndex].deallocate(lowIndex);
            m_high.set(highIndex);
        }

    private:
        engine::BitSet<64> m_high;
        Allocator<262144> m_low[64];
    };

    template<>
    class Allocator<1073741824>
    {
    public:
        const size_t InvalidAllocatorIndex = std::numeric_limits<size_t>::max();

        Allocator()
            : m_high(0xffffffffffffffff)
        {}

        size_t allocate()
        {
            auto next = m_high.begin();
            while (next != m_high.end())
            {
                auto res = m_low[*next].allocate();
                if (res != InvalidAllocatorIndex)
                {
                    return ((*next) * 16777216) + res;
                }
                else
                {
                    m_high.clear(*next);
                    next++;
                }
            }
            return InvalidAllocatorIndex;
        }

        void deallocate(size_t item)
        {
            auto highIndex = item / 16777216;
            auto lowIndex = item - (highIndex * 16777216);
            m_low[highIndex].deallocate(lowIndex);
            m_high.set(highIndex);
        }

    private:
        engine::BitSet<64> m_high;
        Allocator<16777216> m_low[64];
    };

    class ChunkStorage
    {
    private:
        const size_t ChunksPerStorage = ChunkStorageAllocationSize / PreferredChunkSizeBytes;
    public:
        ChunkStorage(
            TypeStorage& typeStorage,
            ArcheTypeStorage& archeTypeStorage)
            : m_typeStorage{ typeStorage }
            , m_archeTypeStorage{ archeTypeStorage }
        {}

        Chunk* allocateChunk(ArcheTypeId archeType)
        {
            auto nextAvailableChunk = m_allocator.allocate();
            ASSERT(nextAvailableChunk != InvalidAllocatorIndex, "Ran out of space for chunks");
            auto storageIndex = nextAvailableChunk / ChunksPerStorage;
            while(storageIndex >= m_allocations.size())
                m_allocations.emplace_back(
                    new ChunkStorageAllocation{
                        ChunkStorageAllocationSize,
                        PreferredChunkSizeBytes,
                        ZeroChunkMemory });
            if (!m_allocations[storageIndex])
                m_allocations[storageIndex] = new ChunkStorageAllocation{
                        ChunkStorageAllocationSize,
                        PreferredChunkSizeBytes,
                        ZeroChunkMemory };

            auto archeTypeInfo = m_archeTypeStorage.archeTypeInfo(archeType);
            return new Chunk(
                m_typeStorage,
                archeTypeInfo,
                StorageAllocation{ m_allocations[storageIndex]->allocate(), nextAvailableChunk, storageIndex });
        }

        void freeChunk(ArcheTypeId archeType, Chunk* chunk)
        {
            StorageAllocation& storAlloc = chunk->m_fromStorageAllocation;
            m_allocator.deallocate(storAlloc.chunkIndex);
            m_allocations[storAlloc.storageIndex]->deallocate(storAlloc.ptr);
            if (m_allocations[storAlloc.storageIndex]->empty())
            {
                delete m_allocations[storAlloc.storageIndex];
                m_allocations[storAlloc.storageIndex] = nullptr;
            }
            delete chunk;
        }
    private:
        TypeStorage& m_typeStorage;
        ArcheTypeStorage& m_archeTypeStorage;
        Allocator<262144> m_allocator;
        engine::vector<ChunkStorageAllocation*> m_allocations;
    };

#if 0
    class ChunkStorage
    {
    public:
        ChunkStorage(
            TypeStorage& typeStorage,
            ArcheTypeStorage& archeTypeStorage)
            : m_typeStorage{ typeStorage }
            , m_archeTypeStorage{ archeTypeStorage }
            , m_currentAllocationIndex{ 0 }
        {
            size_t bytesAllocated = 0;
            while (bytesAllocated < PreallocatedChunkStorageSizeBytes)
            {
                m_storageAllocations.emplace_back(new ChunkStorageAllocation{ ChunkStorageAllocationSize, PreferredChunkSizeBytes, ZeroChunkMemory });
                bytesAllocated += ChunkStorageAllocationSize;
            }
            if (m_storageAllocations.size() == 0)
                m_storageAllocations.emplace_back(new ChunkStorageAllocation{ ChunkStorageAllocationSize, PreferredChunkSizeBytes, ZeroChunkMemory });
        }

        Chunk* allocateChunk(ArcheTypeId archeType)
        {
            return allocateNewChunk(archeType);
        }

        void freeChunk(ArcheTypeId archeType, Chunk* chunk)
        {
            StorageAllocation& chAlloc = chunk->m_fromStorageAllocation;
            chAlloc.storage->deallocate(chAlloc.ptr);
            if (chAlloc.storage->empty()) 
            {
                // switch this loop to unordered_map
                // or maybe allocate using placement new and use the pointer as index
                for (int i = 0; i < m_storageAllocations.size(); ++i)
                {
                    if (m_storageAllocations[i] = chAlloc.storage)
                    {
                        m_storageAllocations.erase(m_storageAllocations.begin() + i);
                        delete chAlloc.storage;

                        if (m_storageAllocations.size() == 0)
                            m_storageAllocations.emplace_back(new ChunkStorageAllocation{ ChunkStorageAllocationSize, PreferredChunkSizeBytes, ZeroChunkMemory });

                        if (m_currentAllocationIndex >= m_storageAllocations.size())
                        {
                            m_currentAllocationIndex = m_storageAllocations.size()-1;
                        }
                        break;
                    }
                }
                
            }
            delete chunk;
        }

    private:
        TypeStorage& m_typeStorage;
        ArcheTypeStorage& m_archeTypeStorage;

        StorageAllocation getStorageAllocation()
        {
            auto chunkAllocation = m_storageAllocations[m_currentAllocationIndex]->allocate();
            if (!chunkAllocation)
            {
                ++m_currentAllocationIndex;
                if(m_currentAllocationIndex >= m_storageAllocations.size())
                    m_storageAllocations.emplace_back(new ChunkStorageAllocation{ ChunkStorageAllocationSize, PreferredChunkSizeBytes, ZeroChunkMemory });
                
                chunkAllocation = m_storageAllocations[m_currentAllocationIndex]->allocate();
            }

            ASSERT(chunkAllocation, "Failed to allocate chunk memory");
            return { chunkAllocation, m_storageAllocations[m_currentAllocationIndex] };
        }

        Chunk* allocateNewChunk(ArcheTypeId archeType)
        {
            auto archeTypeInfo = m_archeTypeStorage.archeTypeInfo(archeType);

            return new Chunk(
                m_typeStorage,
                archeTypeInfo,
                getStorageAllocation());
        }

    private:
        engine::vector<ChunkStorageAllocation*> m_storageAllocations;
        int m_currentAllocationIndex;
        std::mutex m_mutex;
    };
#endif
}
