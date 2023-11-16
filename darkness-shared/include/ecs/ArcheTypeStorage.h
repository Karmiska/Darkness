#pragma once

#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "TypeSort.h"
#include "EcsShared.h"
#include "tools/Debug.h"
#include "tools/ToolsCommon.h"

#include <typeinfo>
#include <cstdint>
#include <limits>
#include <any>
#include <functional>
#include <set>

namespace ecs
{
    template<auto N>
    class BitSet
    {
        static constexpr size_t BitSetDataCount = (size_t)(((float)N / 64.0f)+0.5f);
        static constexpr unsigned long EndIteratorValue = BitSetDataCount * 64;
    public:
        BitSet()
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] = 0;
        }

        BitSet(const BitSet&) = default;
        BitSet(BitSet&&) = default;
        BitSet<N>& operator=(const BitSet<N>& set) = default;
        BitSet<N>& operator=(BitSet<N>&&) = default;

        void set(int index)
        {
            m_data[index / 64] |= (uint64_t)1u << ((uint64_t)index - ((uint64_t)index / (uint64_t)64u));
        }
        
        void clear(int index)
        {
            m_data[index / 64] &= ~((uint64_t)1u << ((uint64_t)index - ((uint64_t)index / (uint64_t)64u)));
        }

        bool get(int index) const
        {
            return m_data[index / 64] & (uint64_t)1u << ((uint64_t)index - ((uint64_t)index / (uint64_t)64u));
        }

        bool operator==(const BitSet& set) const
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                if (m_data[i] != set.m_data[i])
                    return false;
            return true;
        }
        
        bool operator!=(const BitSet& set) const
        {
            return !(*this == set);
        }

        BitSet& operator&=(const BitSet& other) noexcept
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] &= other.m_data[i];

            return *this;
        }
        BitSet& operator|=(const BitSet& other) noexcept
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] |= other.m_data[i];

            return *this;
        }
        BitSet& operator^=(const BitSet& other) noexcept
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] ^= other.m_data[i];

            return *this;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            BitSet res = *this;

            for (int i = 0; i < BitSetDataCount; ++i)
                res.m_data[i] &= other.m_data[i];

            return res;
        }
        BitSet operator|(const BitSet& other) const noexcept
        {
            BitSet res = *this;

            for (int i = 0; i < BitSetDataCount; ++i)
                res.m_data[i] |= other.m_data[i];

            return res;
        }
        BitSet operator^(const BitSet& other) const noexcept
        {
            BitSet res = *this;

            for (int i = 0; i < BitSetDataCount; ++i)
                res.m_data[i] ^= other.m_data[i];

            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint64_t,                   // value_type
            uint64_t,                   // difference_type
            const uint64_t*,            // pointer
            uint64_t>                    // reference
        {
            Iterator(const BitSet* set, uint16_t index)
                : m_set{ set }
                , m_block{ static_cast<unsigned long>(index / 64) }
                , m_index{ index }
                , m_indexInternal{ static_cast<unsigned long>(index - (m_block * 64)) }
            {
                if (index == EndIteratorValue)
                    return;
                this->operator++();
            }

            reference operator*() const { return m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                unsigned long long mask = m_set->m_data[m_block];
                mask = mask >> m_indexInternal << m_indexInternal;
                do
                {
                    if (_BitScanForward64(&m_indexInternal, mask))
                    {
                        m_index = (m_block * 64) + m_indexInternal;
                        ++m_indexInternal;
                        return *this;
                    }
                    ++m_block;
                    if (m_block >= BitSetDataCount)
                    {
                        m_index = EndIteratorValue;
                        return *this;
                    }
                    mask = m_set->m_data[m_block];
                    m_indexInternal = 0;
                } while (true);
                
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const BitSet* m_set;
            uint16_t m_block;

            unsigned long m_index;
            unsigned long m_indexInternal;
        };

        Iterator begin() const
        {
            return Iterator(this, 0);
        }
        Iterator end() const
        {
            return Iterator(this, EndIteratorValue);
        }
    private:
        uint64_t m_data[BitSetDataCount];
    };

    using ArcheTypeSet = BitSet<128>;

    class ArcheType;

    class ArcheTypeStorage
    {
    public:
        static ArcheTypeSet archeTypeSet(const engine::vector<ComponentTypeId>& types)
        {
            ArcheTypeSet set;
            for (auto& t : types)
                set.set(t);
            return set;
        }

        ComponentArcheTypeId archeTypeIdFromSet(const ArcheTypeSet& set)
        {
            for (int i = 0; i < m_archeTypes.size(); ++i)
                if (m_archeTypes[i] == set)
                    return i;
            m_archeTypes.emplace_back(set);
            return m_archeTypes.size() - 1;
        }

        ArcheTypeSet typeSetFromArcheType(ComponentArcheTypeId id)
        {
            return m_archeTypes[id];
        }

        engine::vector<ComponentTypeId> typeIdVectorFromArcheType(ComponentArcheTypeId id)
        {
            engine::vector<ComponentTypeId> res;
            for (auto&& t : m_archeTypes[id])
                res.emplace_back(t);
            return res;
        }

        static ArcheTypeStorage& instance()
        {
            static ArcheTypeStorage archeTypeStorage;
            return archeTypeStorage;
        }

        engine::vector<ComponentArcheTypeId> archeTypesThatContain(const ArcheTypeSet& types)
        {
            engine::vector<ComponentArcheTypeId> res;
            for (int i = 0; i < m_archeTypes.size(); ++i)
                if ((m_archeTypes[i] & types) == types)
                    res.emplace_back(i);
            return res;
        }

    private:
        engine::vector<ArcheTypeSet> m_archeTypes;
    };

    class ArcheType
    {
    public:
        ArcheType()
            : m_id{ InvalidArcheTypeId }
            , m_typeSet{}
        {}

        ArcheType(const engine::vector<ComponentTypeId>& types)
            : m_id{ ArcheTypeStorage::instance().archeTypeIdFromSet(ArcheTypeStorage::archeTypeSet(types)) }
            , m_typeSet{ ArcheTypeStorage::instance().typeSetFromArcheType(m_id) }
        {
        }

        ArcheType(ComponentArcheTypeId id)
            : m_id{ id }
            , m_typeSet{ ArcheTypeStorage::instance().typeSetFromArcheType(id) }
        {
        }

        ArcheType(const ArcheTypeSet& set)
        {
            m_typeSet = set;
            m_id = ArcheTypeStorage::instance().archeTypeIdFromSet(m_typeSet);
        }

        ArcheType(ComponentArcheTypeId id, ComponentTypeId typeId)
        {
            if (id != InvalidArcheTypeId)
            {
                m_typeSet = ArcheTypeStorage::instance().typeSetFromArcheType(id);
                m_typeSet.set(typeId);
                m_id = ArcheTypeStorage::instance().archeTypeIdFromSet(m_typeSet);
            }
            else
            {
                m_typeSet.set(typeId);
                m_id = ArcheTypeStorage::instance().archeTypeIdFromSet(m_typeSet);
            }
        }

        ComponentArcheTypeId id() const
        {
            return m_id;
        }

        bool contains(ComponentTypeId typeId) const
        {
            return m_typeSet.get(typeId);
        }

        const ArcheTypeSet& typeSet() const
        {
            return m_typeSet;
        }

    private:
        ComponentArcheTypeId m_id;
        ArcheTypeSet m_typeSet;
    };
}
