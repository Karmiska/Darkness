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
                size_t _typeSizeBytes,
                size_t alignmentReq,
                std::function<TypeDataBase*(void* ptr, size_t elements)> _create,
                engine::vector<TypeInfo>& typeInfoStorage)
                : id{ _id }
                , typeSizeBytes{ _typeSizeBytes }
                , alignment(alignmentReq)
                , create{ _create }
                
            {
                typeInfoStorage.emplace_back(*this);
            }
            ComponentTypeId id;
            size_t typeSizeBytes;
            size_t alignment;
            std::function<TypeDataBase*(void* ptr, size_t elements)> create;
        };

        template<typename T>
        ComponentTypeId typeId()
        {
            static TypeInfo typeInfo(
                GlobalComponentTypeId++, 
                sizeof(T),
                alignof(T),
                [](void* ptr, size_t elements)->TypeDataBase*
                {
                    return new TypeData<T>(static_cast<T*>(ptr), elements);
                },
                m_typeInfoStorage);
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
