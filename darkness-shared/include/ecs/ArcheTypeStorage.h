#pragma once

#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "TypeSort.h"
#include "EcsShared.h"
#include "tools/Debug.h"
#include "tools/ToolsCommon.h"
#include "containers/BitSet.h"
#include "TypeStorage.h"

#include <typeinfo>
#include <cstdint>
#include <limits>
#include <any>
#include <functional>
#include <set>

namespace ecs
{
    using ArcheTypeSet = engine::BitSet<MaximumEcsTypes>;

    class ArcheType
    {
    public:
        ArcheType()
            : m_id{ InvalidArcheTypeId }
            , m_typeSet{}
        {}

        ArcheType(ComponentArcheTypeId id, ArcheTypeSet typeSet)
            : m_id{ id }
            , m_typeSet{ typeSet }
        {}

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

    class ArcheTypeStorage
    {
    public:
        ArcheTypeStorage(TypeStorage& componentTypeStorage)
            : m_componentTypeStorage{ componentTypeStorage }
        {}

        ArcheType archeType(const engine::vector<ComponentTypeId>& types)
        {
            auto set = archeTypeSet(types);
            return ArcheType(
                archeTypeIdFromSet(set),
                set);
        }

        ArcheType archeType(ComponentArcheTypeId id)
        {
            return ArcheType(id, typeSetFromArcheType(id));
        }

        ArcheType archeType(const ArcheTypeSet& set)
        {
            return ArcheType(archeTypeIdFromSet(set), set);
        }

        ArcheType archeType(ComponentArcheTypeId id, ComponentTypeId typeId)
        {
            if (id != InvalidArcheTypeId)
            {
                auto set = typeSetFromArcheType(id);
                set.set(typeId);
                return ArcheType(archeTypeIdFromSet(set), set);
            }
            else
            {
                ArcheTypeSet set;
                set.set(typeId);
                return ArcheType(archeTypeIdFromSet(set), set);
            }
        }

        ArcheTypeSet archeTypeSet(const engine::vector<ComponentTypeId>& types)
        {
            ArcheTypeSet set;
            for (auto& t : types)
                set.set(t);
            return set;
        }

        ComponentArcheTypeId archeTypeIdFromSet(const ArcheTypeSet& set)
        {
            for (int i = 0; i < m_archeTypes.size(); ++i)
                if (m_archeTypes[i].set == set)
                    return i;
            m_archeTypes.emplace_back(ArcheTypeContainer{ set, archeTypeBytes(set), archeTypeTypeCount(set) });
            return m_archeTypes.size() - 1;
        }

        ArcheTypeSet typeSetFromArcheType(ComponentArcheTypeId id)
        {
            return m_archeTypes[id].set;
        }

        engine::vector<ComponentTypeId> typeIdVectorFromArcheType(ComponentArcheTypeId id)
        {
            engine::vector<ComponentTypeId> res;
            for (auto&& t : m_archeTypes[id].set)
                res.emplace_back(t);
            return res;
        }

        engine::vector<ComponentArcheTypeId> archeTypesThatContain(const ArcheTypeSet& types)
        {
            engine::vector<ComponentArcheTypeId> res;
            for (int i = 0; i < m_archeTypes.size(); ++i)
                if ((m_archeTypes[i].set & types) == types)
                    res.emplace_back(i);
            return res;
        }

        struct ArcheTypeContainer
        {
            ArcheTypeSet set;
            size_t sizeBytes;
            size_t typeCount;
        };

        ArcheTypeContainer archeTypeInfo(ComponentArcheTypeId id)
        {
            return m_archeTypes[id];
        }
    private:
        size_t archeTypeBytes(const ArcheTypeSet& set)
        {
            size_t bytes = 0;
            for (auto&& type : set)
                bytes += m_componentTypeStorage.typeInfo(type).typeSizeBytes;
            return bytes;
        }

        size_t archeTypeTypeCount(const ArcheTypeSet& set)
        {
            size_t count = 0;
            for (auto&& type : set)
                ++count;
            return count;
        }

        TypeStorage& m_componentTypeStorage;
        engine::vector<ArcheTypeContainer> m_archeTypes;
    };

    
}
