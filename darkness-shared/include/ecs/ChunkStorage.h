#pragma once

#include "Chunk.h"
#include "ChunkStorageAllocation.h"
#include "tools/ToolsCommon.h"
#include <atomic>

namespace ecs
{
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
                m_storageAllocations.emplace_back(new ChunkStorageAllocation{ ChunkStorageAllocationSize, ZeroChunkMemory });
                bytesAllocated += ChunkStorageAllocationSize;
            }
            if (m_storageAllocations.size() == 0)
                m_storageAllocations.emplace_back(new ChunkStorageAllocation{ ChunkStorageAllocationSize, ZeroChunkMemory });
        }

        Chunk* allocateChunk(ComponentArcheTypeId archeType)
        {
            if (archeType >= m_freeChunks.size())
                m_freeChunks.resize(archeType + 1);

            auto& chunkList = m_freeChunks[archeType];
            if (chunkList.size())
            {
                auto res = chunkList.back();
                chunkList.pop_back();
                res->m_fromStorageAllocation->allocate();
                return res;
            }

            return allocateNewChunk(archeType);
        }

        void freeChunk(ComponentArcheTypeId archeType, Chunk* chunk)
        {
            ChunkStorageAllocation* chAlloc = chunk->m_fromStorageAllocation;
            chAlloc->deallocate();
            if (chAlloc->allocationCount() == 0)
            {
                std::erase_if(m_freeChunks[chunk->archeType()],
                    [&](Chunk* tmpchunk)
                    {
                        bool match = tmpchunk->m_fromStorageAllocation == chAlloc;
                        if (match)
                            delete tmpchunk;
                        return match;
                    });
                delete chunk;
            }
            else
                m_freeChunks[archeType].emplace_back(chunk);
        }

    private:
        TypeStorage& m_componentTypeStorage;
        ArcheTypeStorage& m_archeTypeStorage;

        void* getStorageAllocation(size_t bytes)
        {
            auto chunkAllocation = m_storageAllocations[m_currentAllocationIndex]->allocate(bytes);
            if (!chunkAllocation)
            {
                ++m_currentAllocationIndex;
                if(m_currentAllocationIndex >= m_storageAllocations.size())
                    m_storageAllocations.emplace_back(new ChunkStorageAllocation{ ChunkStorageAllocationSize, ZeroChunkMemory });
                
                chunkAllocation = m_storageAllocations[m_currentAllocationIndex]->allocate(bytes);
            }

            ASSERT(chunkAllocation, "Failed to allocate chunk memory");
            return chunkAllocation;
        }

        Chunk* allocateNewChunk(ComponentArcheTypeId archeType)
        {
            auto chunk = new Chunk(m_archeTypeStorage, archeType);
            auto chunkBytes = (chunk->elementSizeBytes() * chunk->capacity()) + chunk->typePaddingSizeBytes();
            chunk->initialize(m_componentTypeStorage, getStorageAllocation(chunkBytes), chunkBytes);
            chunk->m_fromStorageAllocation = m_storageAllocations[m_currentAllocationIndex];
            return chunk;
        }

    private:
        engine::vector<engine::vector<Chunk*>> m_freeChunks;
        engine::vector<ChunkStorageAllocation*> m_storageAllocations;
        size_t m_currentAllocationIndex;
        std::mutex m_mutex;
    };
}
