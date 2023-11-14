#pragma once

#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "TypeSort.h"
#include "EcsShared.h"
#include "tools/Debug.h"

#include <typeinfo>
#include <cstdint>
#include <limits>
#include <any>
#include <functional>
#include <set>

namespace ecs
{
    constexpr uint64_t FnvPrime = 1099511628211u;
    constexpr uint64_t FnvOffsetBasis = 14695981039346656037u;
    constexpr uint64_t fnv1aHash(uint64_t value, uint64_t hash = FnvOffsetBasis);

    uint64_t ArcheTypeHash(const std::set<ComponentTypeId>& types);

    class ArcheType;

    class ArcheTypeStorage
    {
    public:
        using HashType = uint64_t;
    public:
        ArcheTypeStorage()
            : m_currentArcheTypeId{ 1 }
        {
            m_archeTypeMap[ArcheTypeHash({})] = ArcheTypeContainer{ EmptyArcheTypeId, {} };
            m_archeTypeIdToContainer.emplace_back(ArcheTypeContainer{ EmptyArcheTypeId, {} });
        }

        static ArcheTypeStorage& instance()
        {
            static ArcheTypeStorage archeTypeStorage;
            return archeTypeStorage;
        }

    public:
        std::vector<ComponentArcheTypeId> archeTypesContain(const std::set<ComponentTypeId>& componentTypes)
        {
            std::vector<ComponentArcheTypeId> result;
            for (auto&& archeTypeContainer : m_archeTypeIdToContainer)
            {
                bool contains = true;
                for (auto&& componentType : componentTypes)
                {
                    if (!archeTypeContainer.typeSet.contains(componentType))
                    {
                        contains = false;
                        break;
                    }
                }
                if (contains)
                    result.emplace_back(archeTypeContainer.id);
            }
            return result;
        }

        const std::set<ComponentTypeId>& typeSetFromArcheTypeId(ComponentArcheTypeId id)
        {
            ASSERT(id < m_archeTypeIdToContainer.size(), "Unknown ArcheTypeId!");
            return m_archeTypeIdToContainer[id].typeSet;
        }

    private:
        ComponentArcheTypeId m_currentArcheTypeId;
        struct ArcheTypeContainer
        {
            ComponentArcheTypeId id;
            std::set<ComponentTypeId> typeSet;
        };
        engine::unordered_map<HashType, ArcheTypeContainer> m_archeTypeMap;
        engine::vector<ArcheTypeContainer> m_archeTypeIdToContainer;

    private:
        friend class ArcheType;
        ComponentArcheTypeId archeTypeIdFromHash(HashType hash)
        {
            auto found = m_archeTypeMap.find(hash);
            if (found != m_archeTypeMap.end())
                return found->second.id;
            return InvalidArcheTypeId;
        }

        ComponentArcheTypeId archeTypeIdFromHash(const std::set<ComponentTypeId>& types, HashType hash)
        {
            auto found = m_archeTypeMap.find(hash);
            if (found != m_archeTypeMap.end())
                return found->second.id;
            else
            {
                ComponentArcheTypeId res = m_currentArcheTypeId++;
                m_archeTypeMap[hash] = ArcheTypeContainer{ res, types };
                if (m_archeTypeIdToContainer.size() <= res)
                    m_archeTypeIdToContainer.resize(res + 1);
                m_archeTypeIdToContainer[res] = ArcheTypeContainer{ res, types };
                return res;
            }
        }
    };

    class ArcheType
    {
    public:
        ArcheType()
            : m_id{ InvalidArcheTypeId }
            , m_typeSet{}
        {}

        ArcheType(const std::set<ComponentTypeId>& types)
            : m_id{ ArcheTypeStorage::instance().archeTypeIdFromHash(types, ArcheTypeHash(types)) }
            , m_typeSet{ types }
        {
        }

        ArcheType(ComponentArcheTypeId id)
            : m_id{ id }
            , m_typeSet{ ArcheTypeStorage::instance().typeSetFromArcheTypeId(id) }
        {
        }

        ComponentArcheTypeId id() const
        {
            return m_id;
        }

        std::set<ComponentTypeId>& typeSet()
        {
            return m_typeSet;
        }
        const std::set<ComponentTypeId>& typeSet() const
        {
            return m_typeSet;
        }

    private:
        ComponentArcheTypeId m_id;
        std::set<ComponentTypeId> m_typeSet;
    };
}
