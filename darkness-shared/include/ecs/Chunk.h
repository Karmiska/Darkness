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

    struct StorageAllocation
    {
        void* ptr;
        ChunkStorageAllocation* storage;
    };

    class Chunk
    {
        
    public:
        Chunk()
            : m_elements{ 0 }
            , m_componentTypeIds{}
            , m_componentData{}
            , m_entityIds{ nullptr }
            , m_used{ 0 }
            , m_fromStorageAllocation{}
            , m_archeType{}
        {}

        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;

        Chunk(Chunk&& chunk) noexcept
            : m_elements{ 0 }
            , m_componentTypeIds{}
            , m_componentData{}
            , m_entityIds{ nullptr }
            , m_used{ 0 }
            , m_fromStorageAllocation{}
            , m_archeType{}
        {
            std::swap(m_elements, chunk.m_elements);
            std::swap(m_componentTypeIds, chunk.m_componentTypeIds);
            std::swap(m_componentData, chunk.m_componentData);
            std::swap(m_entityIds, chunk.m_entityIds);
            std::swap(m_used, chunk.m_used);
            std::swap(m_fromStorageAllocation, chunk.m_fromStorageAllocation);
            std::swap(m_archeType, chunk.m_archeType);
        }

        Chunk& operator=(Chunk&& chunk) noexcept
        {
            std::swap(m_elements, chunk.m_elements);
            std::swap(m_componentTypeIds, chunk.m_componentTypeIds);
            std::swap(m_componentData, chunk.m_componentData);
            std::swap(m_entityIds, chunk.m_entityIds);
            std::swap(m_used, chunk.m_used);
            std::swap(m_fromStorageAllocation, chunk.m_fromStorageAllocation);
            std::swap(m_archeType, chunk.m_archeType);
        }

        Chunk(
            TypeStorage& componentTypeStorage,
            const ArcheTypeStorage::ArcheTypeInfo& archeTypeInfo,
            StorageAllocation allocation)
            : m_used{ 0 }
            , m_fromStorageAllocation{ nullptr }
            , m_archeType{ archeTypeInfo.id }
        {
            m_componentTypeIds = archeTypeInfo.set;
            m_componentData.reserve(archeTypeInfo.typeCount);
            m_elements = archeTypeInfo.elementsInChunk;

            auto ptr = reinterpret_cast<uintptr_t>(allocation.ptr);
            m_entityIds = reinterpret_cast<EntityId*>(reinterpret_cast<uint8_t*>(allocation.ptr) + PreferredChunkSizeBytes - (sizeof(EntityId) * m_elements));
            
            for (auto&& type : archeTypeInfo.set)
            {
                auto typeInfo = componentTypeStorage.typeInfo(type);
                auto alignment = std::max(typeInfo.alignment, ChunkDataAlignment);

                ptr = roundUpToMultiple(ptr, alignment);
                m_componentData.emplace_back(typeInfo.create(reinterpret_cast<void*>(ptr), m_elements));
                ptr += std::max(typeInfo.typeSizeBytes, typeInfo.alignment) * m_elements;
            }

            ASSERT(ptr <= reinterpret_cast<uintptr_t>(allocation.ptr) + PreferredChunkSizeBytes, "Wrote over the chunk allocation");

            m_fromStorageAllocation = allocation;
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
            auto res = m_used++;
            ASSERT(res < m_elements, "Tried to allocate more entities from Chunk that it has");
            return res;
        }

        void swap(uint64_t a, uint64_t b) noexcept
        {
            for (auto&& type : m_componentData)
                type->swap(a, b);
            std::swap(m_entityIds[a], m_entityIds[b]);
        }

        void freeLast()
        {
            // this trategy only works if the entity being removed is always the last
            // (which it currently is, as entities getting removed are moved to last)
            --m_used;
        }

        template<typename T>
        typename std::remove_reference<T>::type* componentDataPointer(TypeId componentTypeId)
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

        void* componentDataPointer(TypeId componentTypeId)
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

        ArcheTypeId archeType() const
        {
            return m_archeType;
        }

        EntityId* entities()
        {
            return m_entityIds;
        }
    private:
        size_t m_elements;
        ArcheTypeSet m_componentTypeIds;
        engine::vector<TypeDataBase*> m_componentData;
        EntityId* m_entityIds;
        size_t m_used;

    public:

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
    private:
        friend class ChunkStorage;
        StorageAllocation m_fromStorageAllocation;
        ArcheTypeId m_archeType;
    };
    
}
