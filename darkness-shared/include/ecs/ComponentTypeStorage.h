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
#include <new>

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
        ComponentData(T* data, size_t size)
            : m_data{ data }
            , m_size{ size }
        {
        }

        ~ComponentData()
        {
            if (!(std::is_standard_layout<T>().value && std::is_trivial<T>().value))
                for (size_t i = 0; i < m_size; ++i)
                    m_data[i].~T();
        }

        T* data() { return m_data; }
        const T* data() const { return m_data; }
        void* rawData() override { return m_data; }

        size_t size() const { return m_size; }

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

    private:
        T* m_data;
        size_t m_size;
    };

    class ComponentTypeStorage
    {
    public:
        ComponentTypeStorage()
        {};

        struct TypeInfo
        {
            TypeInfo(
                ComponentTypeId _id,
                std::function<ComponentDataBase*(void* ptr, size_t elements)> _create,
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
            std::function<ComponentDataBase*(void* ptr, size_t elements)> create;
        };

        template<typename T>
        ComponentTypeId typeId()
        {
            static TypeInfo typeInfo(
                GlobalComponentTypeId++, 
                [](void* ptr, size_t elements)->ComponentDataBase*
                {
                    if(!(std::is_standard_layout<T>().value && std::is_trivial<T>().value))
                        auto comdataptr = new (ptr) T[elements];
                    auto res = new ComponentData<T>(static_cast<T*>(ptr), elements);
                    return res;
                },
                m_typeInfoStorage,
                sizeof(T));
            return typeInfo.id;
        }

        const TypeInfo& typeInfo(ComponentTypeId id) const
        {
            return m_typeInfoStorage[id];
        }

    private:
        engine::vector<TypeInfo> m_typeInfoStorage;

    };

}
