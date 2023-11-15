#pragma once

#include "containers/vector.h"
#include "tools/ToolsCommon.h"
#include "ComponentTypeStorage.h"
#include "Entity.h"
#include <stack>

namespace ecs
{
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
            m_componentTypeIds = ArcheTypeStorage::instance().typeIdVectorFromArcheType(archeType);
            uint32_t combinedTypeBytes = 0;
            
            for (auto&& type : m_componentTypeIds)
            {
                auto typeInfo = ComponentTypeStorage::typeInfo(type);
                combinedTypeBytes += typeInfo.typeSizeBytes;
            }

            // not sure why his was here?
            //combinedTypeBytes += sizeof(Entity);
            
            m_elements = PreferredChunkSizeBytes / combinedTypeBytes;
            
            for (auto&& type : m_componentTypeIds)
            {
                auto typeInfo = ComponentTypeStorage::typeInfo(type);
                m_componentData.emplace_back(typeInfo.create(m_elements));
            }

            m_occupancy.resize(roundUpToMultiple(m_elements / 64u, 64));
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
        }

        void swap(uint64_t a, uint64_t b)
        {
            for (auto&& type : m_componentData)
                static_cast<ComponentDataBase*>(type)->swap(a, b);
        }

        void free(uint64_t id)
        {
            uint16_t ocp = id / 64;
            uint16_t r = id - (ocp * 64);
            m_occupancy[ocp] &= ~(1u << (uint64_t)r);
            --m_used;
        }

        bool isSet(uint64_t id) const
        {
            uint16_t ocp = id / 64;
            uint16_t r = id - (ocp * 64);
            return m_occupancy[ocp] & (1u << (uint64_t)r);
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
            for (int i = 0; i < srcChunk.m_componentTypeIds.size(); ++i)
            {
                auto typeId = srcChunk.m_componentTypeIds[i];
                for (int a = 0; a < m_componentTypeIds.size(); ++a)
                {
                    if (typeId == m_componentTypeIds[a])
                    {
                        m_componentData[a]->copy(srcChunk.m_componentData[i], srcIndex, dstIndex, elements);
                        break;
                    }
                }
            }
        }

    private:
        size_t m_elements;
        engine::vector<ComponentTypeId> m_componentTypeIds;
        engine::vector<ComponentDataBase*> m_componentData;

        // should be optimized as bitset
        std::vector<uint64_t> m_occupancy;
        size_t m_used;

    public:
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

    };

    //class ChunkStorage
    //{
    //public:
    //    Chunk createChunk(ComponentArcheTypeId archeType)
    //    {
    //        if (archeType >= m_freeChunks.size())
    //            m_freeChunks.resize(archeType + 1);
    //
    //        auto& chunkList = m_freeChunks[archeType];
    //        if (chunkList.size())
    //        {
    //            auto res = chunkList.top();
    //            chunkList.pop();
    //            return res;
    //        }
    //
    //    }
    //private:
    //    engine::vector<std::stack<Chunk>> m_freeChunks;
    //};
}
