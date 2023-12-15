#pragma once

#include "containers/vector.h"
#include "EcsShared.h"
#include "TypeData.h"

#include <functional>

namespace ecs
{
    class TypeStorage
    {
    public:
        TypeStorage()
        {};

        struct TypeInfo
        {
            TypeInfo(
                ComponentTypeId _id,
                std::function<TypeDataBase*(void* ptr, size_t elements)> _create,
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
            std::function<TypeDataBase*(void* ptr, size_t elements)> create;
        };

        template<typename T>
        ComponentTypeId typeId()
        {
            static TypeInfo typeInfo(
                GlobalComponentTypeId++, 
                [](void* ptr, size_t elements)->TypeDataBase*
                {
                    return new TypeData<T>(static_cast<T*>(ptr), elements);
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
        static engine::vector<TypeInfo> m_typeInfoStorage;

    };

}
