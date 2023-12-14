#pragma once

#include "containers/vector.h"
#include "tools/ToolsCommon.h"
#include "TypeStorage.h"
#include "EcsShared.h"
#include "ChunkStorageAllocation.h"
#include "ArcheTypeStorage.h"
#include "Entity.h"
#include <stack>
#include <mutex>
#include <execution>

namespace ecs
{
#define NOHOLES

    // chunks have a maximum index of PreferredChunkSizeBytes if only one
    // element and its size is one byte. we store free chunk indexes in uint16.
    static_assert(PreferredChunkSizeBytes <= UINT16_MAX+1);

    class Chunk
    {
        
    public:
        Chunk(
            ArcheTypeStorage& archeTypeStorage, 
            ComponentArcheTypeId archeType)
            : m_used{ 0 }
            , m_fromStorageAllocation{ nullptr }
            , m_archeType{ archeType }
        {
            auto archeTypeInfo = archeTypeStorage.archeTypeInfo(archeType);
            m_componentTypeIds = archeTypeInfo.set;
            m_componentData.reserve(archeTypeInfo.typeCount);
            m_typePaddingSizeBytes = archeTypeInfo.typeCount * ChunkDataAlignment;
            auto chunkSizeForData = PreferredChunkSizeBytes - m_typePaddingSizeBytes;

            m_elements = chunkSizeForData / archeTypeInfo.sizeBytes;
            m_elementSizeBytes = archeTypeInfo.sizeBytes;
        }

        // storageBytes is 64 bytes aligned ptr.
        // it contains enough space for (capacity() * elementSizeBytes()) + typePaddingSizeBytes()
        // typePaddingSizeBytes() = 64 bytes after each component data
        void initialize(TypeStorage& componentTypeStorage, void* storageBytes, size_t bytes)
        {
            auto ptr = reinterpret_cast<uintptr_t>(storageBytes);
            m_entityIds = reinterpret_cast<EntityId*>(reinterpret_cast<uint8_t*>(storageBytes) + bytes - (sizeof(EntityId) * m_elements));
            for (auto&& type : m_componentTypeIds)
            {
                auto typeInfo = componentTypeStorage.typeInfo(type);
                auto typeBytes = typeInfo.typeSizeBytes * m_elements;
                m_componentData.emplace_back(typeInfo.create(reinterpret_cast<void*>(ptr), m_elements));
                ptr += typeBytes;
                ptr = roundUpToMultiple(ptr, ChunkDataAlignment);
            }

#ifndef NOHOLES
            m_occupancy.resize(roundUpToMultiple(m_elements / 64u, 64));
#endif
        }

        size_t typePaddingSizeBytes() const
        {
            return m_typePaddingSizeBytes;
        }

        size_t elementSizeBytes() const
        {
            return m_elementSizeBytes;
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
            return m_elements - m_used;
        }

        bool full() const
        {
            return m_elements == m_used;
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
            auto res = m_used++;
            ASSERT(res < m_elements+1, "wut?");
            return res;
#endif
        }

        void swap(uint64_t a, uint64_t b)
        {
            for (auto&& type : m_componentData)
                type->swap(a, b);
            auto id = m_entityIds[a];
            m_entityIds[a] = m_entityIds[b];
            m_entityIds[b] = id;
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
                    return static_cast<TypeData<typename std::remove_reference<T>::type>*>(m_componentData[index])->data();
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
                    return m_componentData[index]->rawData();
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
            for (int i = 0; i < elements; ++i)
                m_entityIds[dstIndex + i] = srcChunk.m_entityIds[srcIndex + i];
        }

        ComponentArcheTypeId archeType() const
        {
            return m_archeType;
        }

        EntityId* entities()
        {
            return m_entityIds;
        }
    private:
        size_t m_elements;
        size_t m_elementSizeBytes;
        size_t m_typePaddingSizeBytes;
        ArcheTypeSet m_componentTypeIds;
        engine::vector<TypeDataBase*> m_componentData;
        EntityId* m_entityIds;

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
    private:
        friend class ChunkStorage;
        ChunkStorageAllocation* m_fromStorageAllocation;
        ComponentArcheTypeId m_archeType;
    };
    
}
