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
}
