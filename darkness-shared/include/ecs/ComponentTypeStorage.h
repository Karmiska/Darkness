#pragma once

#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "TypeSort.h"
#include "EcsShared.h"
#include "ArcheTypeStorage.h"
#include "tools/Debug.h"

#include <typeinfo>
#include <cstdint>
#include <limits>
#include <any>
#include <functional>
#include <set>

namespace ecs
{
    class ComponentDataBase
    {
    public:
        virtual ~ComponentDataBase() {}
        virtual void* rawData() = 0;
        virtual void swap(uint64_t a, uint64_t b) noexcept = 0;
        virtual void copy(ComponentDataBase* src, uint64_t srcIndex, uint64_t dstIndex, size_t elements) noexcept = 0;
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

        void swap(uint64_t a, uint64_t b) noexcept override
        {
            std::swap(m_data[a], m_data[b]);
        }

        void copy(ComponentDataBase* src, uint64_t srcIndex, uint64_t dstIndex, size_t elements) noexcept override
        {
            ComponentData<T>& srcCom = *static_cast<ComponentData<T>*>(src);
            for (int i = 0; i < elements; ++i)
            {
                m_data[dstIndex + i] = srcCom.m_data[srcIndex + i];
            }
        }

        void* rawData() override { return m_data.data(); }
    private:
        engine::vector<T> m_data;
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
