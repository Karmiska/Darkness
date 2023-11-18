#pragma once

#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "TypeSort.h"
#include "EcsShared.h"
#include "tools/Debug.h"
#include "tools/ToolsCommon.h"
#include "containers/BitSet.h"

#include <typeinfo>
#include <cstdint>
#include <limits>
#include <any>
#include <functional>
#include <set>

namespace ecs
{
    using ArcheTypeSet = engine::BitSet<128>;

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
