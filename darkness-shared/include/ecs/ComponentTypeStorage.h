#pragma once

#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "TypeSort.h"
#include "tools/Debug.h"

#include <typeinfo>
#include <cstdint>
#include <limits>
#include <any>
#include <functional>
#include <set>

namespace ecs
{
    using ComponentTypeId = uint64_t;
    using ComponentArcheTypeId = uint64_t;
    const ComponentTypeId InvalidTypeId = std::numeric_limits<uint64_t>::max();
    const ComponentArcheTypeId InvalidArcheTypeId = 0xffffff;
    const ComponentArcheTypeId EmptyArcheTypeId = 0x0;
    static ComponentTypeId GlobalComponentTypeId = 0;
    //static ComponentArcheTypeId GlobalArcheTypeId = 0;

    template<typename T>
    class ComponentTypeContainer
    {};

    constexpr uint64_t FnvPrime = 1099511628211u;
    constexpr uint64_t FnvOffsetBasis = 14695981039346656037u;
    constexpr uint64_t fnv1aHash(uint64_t value, uint64_t hash = FnvOffsetBasis);
    constexpr uint64_t fnv1aHash(uint64_t value, uint64_t hash)
    {
        hash ^= (value & 0xff);
        hash *= FnvPrime;
        hash ^= (value & 0xff00) >> 8;
        hash *= FnvPrime;
        hash ^= (value & 0xff0000) >> 16;
        hash *= FnvPrime;
        hash ^= (value & 0xff000000) >> 24;
        hash *= FnvPrime;

        hash ^= (value & 0xff00000000) >> 32;
        hash *= FnvPrime;
        hash ^= (value & 0xff0000000000) >> 40;
        hash *= FnvPrime;
        hash ^= (value & 0xff000000000000) >> 48;
        hash *= FnvPrime;
        hash ^= (value & 0xff00000000000000) >> 56;
        hash *= FnvPrime;
        return hash;
    }

    class ComponentDataBase
    {
    public:
        virtual ~ComponentDataBase() {}
    };

    template<typename T>
    class ComponentData : public ComponentDataBase
    {
    public:
        T* data() { return m_data.data(); }
        const T* data() const { return m_data.data(); }

        engine::vector<T>& storage() { return m_data; }
        const engine::vector<T>& storage() const { return m_data; }

        void resize(size_t size)
        {
            m_data.resize(size);
        }
    private:
        engine::vector<T> m_data;
    };

    uint64_t ArcheTypeHash(const std::set<ComponentTypeId>& types);

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

    class ComponentTypeStorage
    {
    public:

        struct TypeInfo
        {
            TypeInfo(
                ComponentTypeId _id,
                std::function<ComponentDataBase*(size_t size)> _create,
                engine::vector<TypeInfo>& typeInfoStorage,
                uint32_t _typeSizeBytes)
                : id{ _id }
                , create{ _create }
                , typeSizeBytes{ _typeSizeBytes }
            {
                typeInfoStorage.emplace_back(*this);
            }
            ComponentTypeId id;
            uint32_t typeSizeBytes;
            std::function<ComponentDataBase*(size_t size)> create;
        };

        template<typename T>
        static ComponentTypeId typeId()
        {
            static TypeInfo typeInfo(
                GlobalComponentTypeId++, 
                [](size_t size)->ComponentDataBase* { auto res = new ComponentData<T>(); res->resize(size); return res; },
                ComponentTypeStorage::instance().m_typeInfoStorage,
                sizeof(T));
            return typeInfo.id;
        }

        static ArcheType archeTypeId(const std::set<ComponentTypeId>& types)
        {
            return ArcheType(types);
        }

#ifdef STATIC_ARCHETYPE_IMPL
        struct ArcheTypeInfo
        {
            ArcheTypeInfo(
                ComponentArcheTypeId _id,
                engine::vector<ArcheTypeInfo>& typeInfoStorage)
                : id{ _id }
            {
                typeInfoStorage.emplace_back(*this);
            }
            ComponentArcheTypeId id;
        };

    private:
        template<typename T>
        static ComponentArcheTypeId archetypeInternalId()
        {
            static ComponentArcheTypeId id(GlobalArcheTypeId++);
            return id;
        }
    public:
        template<typename T>
        static std::tuple<typename std::remove_reference<T>::type> packCom()
        {
            return std::make_tuple<typename std::remove_reference<T>::type>(std::remove_reference<T>::type());
        }

        template <typename T, typename Arg, typename... Args>
        static std::tuple<typename std::remove_reference<T>::type, typename std::remove_reference<Arg>::type, Args...> packCom()
        {
            return std::tuple_cat(
                std::make_tuple<typename std::remove_reference<T>::type>(std::remove_reference<T>::type()),
                packCom<Arg, Args...>());
        }

        template<typename... Args>
        static ComponentArcheTypeId archetypeId()
        {
            std::any test = packCom<Args...>();//std::tuple<Args...>();
            LOG("%s", test.type().name());
            auto id = archetypeInternalId<instantiate_t<SortedTypeContainer, sorted_list_t<list<Args...>>>>();
            static ArcheTypeInfo archetypeInfo(
                id,
                ComponentTypeStorage::instance().m_archeTypeInfoStorage);

            return id;
        }
#endif

        static TypeInfo& typeInfo(ComponentTypeId id)
        {
            return ComponentTypeStorage::instance().m_typeInfoStorage[id];
        }

#ifdef STATIC_ARCHETYPE_IMPL
        static ArcheTypeInfo& archeTypeInfo(ComponentArcheTypeId id)
        {
            return ComponentTypeStorage::instance().m_archeTypeInfoStorage[id];
        }
#endif

#if 0
        //ARCHETYPE_HASHED
        static ComponentArcheTypeId archeTypeId(const engine::vector<ComponentTypeId>& typeIds)
        {
            if (typeIds.empty())
                return {};
        
            uint64_t hash = fnv1aHash(*typeIds.begin());
            for (auto t = typeIds.begin() + 1; t != typeIds.end(); ++t)
                hash ^= fnv1aHash(*t);
        
            auto& storage = ComponentTypeStorage::instance().m_archeTypeStorage;
            auto existing = storage.find(hash);
            if (existing != storage.end())
            {
                return existing->second.id;
            }
            else
            {
                auto result = ComponentTypeStorage::instance().m_consistentArchetypeId++;
                storage[hash] = { hash, result, typeIds };
                return result;
            }
        }
        
        static engine::vector<ComponentTypeId> typeIds(ComponentArcheTypeId archeType)
        {
            ComponentTypeStorage& storage = ComponentTypeStorage::instance();
            for (auto&& pair : storage.m_archeTypeStorage)
            {
                if (pair.second.id == archeType)
                    return pair.second.components;
            }
            return {};
        }
    private:
        struct ArcheTypeInfo
        {
            uint64_t hash;
            ComponentArcheTypeId id;
            engine::vector<ComponentTypeId> components;
        };
        engine::unordered_map<uint64_t, ArcheTypeInfo> m_archeTypeStorage;
        ComponentArcheTypeId m_consistentArchetypeId;

#endif

    private:
        ComponentTypeStorage()
            //: m_consistentArchetypeId{ 0 }
        {}

        static ComponentTypeStorage& instance()
        {
            static ComponentTypeStorage storage;
            return storage;
        }

        engine::vector<TypeInfo> m_typeInfoStorage;

#ifdef STATIC_ARCHETYPE_IMPL
        engine::vector<ArcheTypeInfo> m_archeTypeInfoStorage;
#endif
    };

#ifdef STATIC_ARCHETYPE_IMPL
    struct InvalidArcheType{};
    static auto InvalidArchetypeId = ComponentTypeStorage::archetypeId<InvalidArcheType>();
#endif
}
