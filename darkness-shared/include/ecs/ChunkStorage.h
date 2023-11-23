#pragma once

#include "ComponentData.h"
#include "tools/ToolsCommon.h"
#include <atomic>

#define MULTITHREADED_PREWARM

namespace ecs
{
    struct Range
    {
    public:
        Range(uint64_t range)
            : m_range{ range }
        {}

        struct Iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = uint64_t;
            using pointer = uint64_t*;
            using reference = uint64_t;

            Iterator()
                : m_index{ 0 }
            {}

            Iterator(uint64_t index)
                : m_index{ index }
            {}

            reference operator*() const { return m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                ++m_index;
                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };

        private:
            uint64_t m_index;
        };

        Iterator begin() const
        {
            return Iterator(0);
        }
        Iterator end() const
        {
            return Iterator(m_range);
        }
    private:
        uint64_t m_range;
    };


    class ChunkStorage
    {
    private:
        ComponentTypeStorage& m_componentTypeStorage;
        ArcheTypeStorage& m_archeTypeStorage;
        void* m_storageAllocation;
        

#ifndef MULTITHREADED_PREWARM
        uintptr_t m_inUse;
#else
        std::atomic<uintptr_t> m_inUse;
#endif

        void* allocate(size_t bytes)
        {
#ifndef MULTITHREADED_PREWARM
            uintptr_t alignedPtr = roundUpToMultiple(m_inUse, ChunkDataAlignment);
            m_inUse += (alignedPtr - m_inUse) + bytes;
#else
            uintptr_t current = m_inUse.load(std::memory_order_relaxed);
            uintptr_t alignedPtr = roundUpToMultiple(current, ChunkDataAlignment);
            uintptr_t next = (alignedPtr - current) + bytes;

            while (!std::atomic_compare_exchange_weak_explicit(
                &m_inUse, &current, next,
                std::memory_order_release, std::memory_order_relaxed))
            {
                current = m_inUse.load(std::memory_order_relaxed);
                alignedPtr = roundUpToMultiple(current, ChunkDataAlignment);
                next = (alignedPtr - current) + bytes;
            };
#endif
            return static_cast<uint8_t*>(m_storageAllocation) + alignedPtr;
        }

        Chunk* allocateNewChunk(ComponentArcheTypeId archeType)
        {
            auto chunk = new Chunk(m_archeTypeStorage, archeType);
            auto chunkBytes = (chunk->elementSizeBytes() * chunk->capacity()) + chunk->typePaddingSizeBytes();
            chunk->initialize(m_componentTypeStorage, allocate(chunkBytes), chunkBytes);
            return chunk;
        }
    public:
        ChunkStorage(
            ComponentTypeStorage& componentTypeStorage, 
            ArcheTypeStorage& archeTypeStorage)
            : m_componentTypeStorage{ componentTypeStorage }
            , m_archeTypeStorage{ archeTypeStorage }
            , m_storageAllocation{ nullptr }
            , m_inUse{ 0 }
        {
            m_storageAllocation = _aligned_malloc(6ull * 1024ull * 1024ull * 1024ull, ChunkDataAlignment);
        }

        ~ChunkStorage()
        {
            _aligned_free(m_storageAllocation);
        }

        Chunk* allocateChunk(ComponentArcheTypeId archeType)
        {
            if (archeType >= m_freeChunks.size())
                m_freeChunks.resize(archeType + 1);

            auto& chunkList = m_freeChunks[archeType];
            if (chunkList.size())
            {
#ifndef MULTITHREADED_PREWARM
                auto res = chunkList.top();
                chunkList.pop();
#else
                auto res = chunkList.back();
                chunkList.pop_back();
#endif
                return res;
            }

            return allocateNewChunk(archeType);
        }

        void freeChunk(ComponentArcheTypeId archeType, Chunk* chunk)
        {
#ifndef MULTITHREADED_PREWARM
            m_freeChunks[archeType].push(chunk);
#else
            m_freeChunks[archeType].emplace_back(chunk);
#endif
        }

        void reserve(ComponentArcheTypeId archeType, size_t chunks)
        {
            if (archeType >= m_freeChunks.size())
                m_freeChunks.resize(archeType + 1);

#ifndef MULTITHREADED_PREWARM
            for (int i = m_freeChunks.size(); i < chunks; ++i)
            {
                m_freeChunks[archeType].push(allocateNewChunk(archeType));
            }
#else
            m_freeChunks[archeType].resize(chunks);
            Range range(chunks);
            std::for_each(
                std::execution::par_unseq,
                range.begin(),
                range.end(),
                [&](auto id)
                {
                    m_freeChunks[archeType][id] = allocateNewChunk(archeType);
                });
#endif
        }
    private:
#ifndef MULTITHREADED_PREWARM
        engine::vector<std::stack<Chunk*>> m_freeChunks;
#else
        engine::vector<std::vector<Chunk*>> m_freeChunks;
#endif
        std::mutex m_mutex;
    };
}
