#pragma once

#include "containers/vector.h"
#include "tools/ToolsCommon.h"
#include "ComponentTypeStorage.h"
#include "Entity.h"
#include <stack>
#include <mutex>
#include <execution>

namespace ecs
{
#define NOHOLES

    constexpr size_t PreferredChunkSizeBytes = 64 * 1024;

    // chunks have a maximum index of PreferredChunkSizeBytes if only one
    // element and its size is one byte. we store free chunk indexes in uint16.
    static_assert(PreferredChunkSizeBytes <= UINT16_MAX+1);

    class Chunk
    {
    public:
        Chunk(ComponentArcheTypeId archeType)
            : m_used{ 0 }
        {
            m_componentTypeIds = ArcheTypeStorage::instance().typeSetFromArcheType(archeType);
            uint32_t combinedTypeBytes = 0;
            
            for (auto&& type : m_componentTypeIds)
            {
                auto typeInfo = ComponentTypeStorage::typeInfo(type);
                combinedTypeBytes += typeInfo.typeSizeBytes;
            }
            
            m_elements = PreferredChunkSizeBytes / combinedTypeBytes;

            for (auto&& type : m_componentTypeIds)
            {
                auto typeInfo = ComponentTypeStorage::typeInfo(type);
                m_componentData.emplace_back(typeInfo.create(m_elements));
            }

#ifndef NOHOLES
            m_occupancy.resize(roundUpToMultiple(m_elements / 64u, 64));
#endif
        }

        size_t size() const
        {
            return m_used;
        }

        size_t capacity() const
        {
            return m_elements;
        }

        size_t available() const
        {
            return capacity() - size();
        }

        bool empty() const
        {
            return m_used == 0;
        }

        uint64_t allocate()
        {
#ifndef NOHOLES
            for (int i = 0; i < m_occupancy.size(); ++i)
            {
                auto& ocp = m_occupancy[i];
                if (ocp != 0xffffffffffffffff)
                {
                    unsigned long index = 0;
                    unsigned long long mask = ~ocp;
                    if (_BitScanForward64(&index, mask))
                    {
                        ocp |= ((uint64_t)1u << (uint64_t)index);
                        ++m_used;
                        return (i * 64) + index;
                    }
                    else
                    {
                        LOG("fail");
                    }
                }
            }
            ASSERT(false, "");
            return 0;
#else
            return m_used++;
#endif
        }

        void swap(uint64_t a, uint64_t b)
        {
            for (auto&& type : m_componentData)
                static_cast<ComponentDataBase*>(type)->swap(a, b);
        }

        void free(uint64_t id)
        {
#ifndef NOHOLES
            uint16_t ocp = id / 64;
            uint16_t r = id - (ocp * 64);
            m_occupancy[ocp] &= ~(1u << (uint64_t)r);
            --m_used;
#else
            --m_used;
#endif
        }

        bool isSet(uint64_t id) const
        {
#ifndef NOHOLES
            uint16_t ocp = id / 64;
            uint16_t r = id - (ocp * 64);
            return m_occupancy[ocp] & (1u << (uint64_t)r);
#else
            return id < m_used;
#endif
        }

        template<typename T>
        typename std::remove_reference<T>::type* componentDataPointer(ComponentTypeId componentTypeId)
        {
            int index = 0;

            for (auto&& type : m_componentTypeIds)
            {
                if (type == componentTypeId)
                    return static_cast<ComponentData<typename std::remove_reference<T>::type>*>(m_componentData[index])->data();
                ++index;
            }
            return nullptr;
        }

        void* componentDataPointer(ComponentTypeId componentTypeId)
        {
            int index = 0; 

            for (auto&& type : m_componentTypeIds)
            {
                if (type == componentTypeId)
                    return static_cast<ComponentDataBase*>(m_componentData[index])->rawData();
                ++index;
            }
            return nullptr;
        }

        void copy(const Chunk& srcChunk, uint64_t srcIndex, uint64_t dstIndex, size_t elements)
        {
            int i = 0;
            for (auto srcTypeId : srcChunk.m_componentTypeIds)
            {
                int a = 0;
                for (auto dstTypeId : m_componentTypeIds)
                {
                    if (srcTypeId == dstTypeId)
                    {
                        m_componentData[a]->copy(srcChunk.m_componentData[i], srcIndex, dstIndex, elements);
                        break;
                    }
                    ++a;
                }
                ++i;
            }
        }

    private:
        size_t m_elements;
        ArcheTypeSet m_componentTypeIds;
        engine::vector<ComponentDataBase*> m_componentData;

        // should be optimized as bitset
#ifndef NOHOLES
        std::vector<uint64_t> m_occupancy;
#endif
        size_t m_used;

    public:
#ifndef NOHOLES
        struct Iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = uint16_t;
            using pointer = uint16_t*;
            using reference = uint16_t;

            Iterator(const Chunk* chunk, uint16_t index)
                : m_chunk{ chunk }
                , m_index{ index }
            {}

            reference operator*() const { return m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                auto cont = m_index / 64;
                auto bit = m_index - (cont * 64);
                unsigned long index = bit + 1;
                if (index > 63)
                {
                    index = 0;
                    ++cont;
                }
                for (int i = cont; i < m_chunk->m_occupancy.size(); ++i)
                {
                    auto ocp = (m_chunk->m_occupancy[i] >> (uint64_t)index) << (uint64_t)index;
                    if (ocp != 0)
                    {
                        
                        if (_BitScanForward64(&index, ocp))
                        {
                            m_index = (i * 64) + index;
                            return *this;
                        }
                    }
                    index = 0;
                }

                m_index = m_chunk->m_occupancy.size() * 64;
                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };

        private:
            const Chunk* m_chunk;
            uint16_t m_index;
        };

        Iterator begin() const
        {
            for (int i = 0; i < m_occupancy.size(); ++i)
            {
                auto& ocp = m_occupancy[i];
                if (ocp != 0)
                {
                    unsigned long index = 0;
                    if (_BitScanForward64(&index, ocp))
                    {
                        return Iterator(this, (i * 64) + index);
                    }
                }
            }
            return Iterator(this, 0);
        }
        Iterator end() const
        {
            return Iterator(this, m_occupancy.size() * 64);
        }
#else
        struct Iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = uint16_t;
            using pointer = uint16_t*;
            using reference = uint16_t;

            Iterator(uint16_t index)
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
            uint16_t m_index;
        };

        Iterator begin() const
        {
            return Iterator(0);
        }
        Iterator end() const
        {
            return Iterator(m_used);
        }
#endif
    };

#undef MULTITHREADED_PREWARM

    class ChunkStorage
    {
    private:
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
                    : m_index{0}
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
    public:
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
    
            return new Chunk(archeType);
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
                m_freeChunks[archeType].push(new Chunk(archeType));
#else
            m_freeChunks[archeType].resize(chunks);
            Range range(chunks);
            std::for_each(
                std::execution::par_unseq,
                range.begin(),
                range.end(),
                [&](auto id)
            {
                m_freeChunks[archeType][id] = new Chunk(archeType);
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
