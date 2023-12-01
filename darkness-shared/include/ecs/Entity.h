#pragma once

#include "TypeStorage.h"
#include "ArcheTypeStorage.h"
#include <cstdint>
#include <tuple>

namespace ecs
{
    constexpr uint64_t EntityIdArcheTypeMask = 0xffffff0000000000;
    constexpr uint64_t EntityIdChunkMask =     0x000000ffffff0000;
    constexpr uint64_t EntityIdEntityMask =    0x000000000000ffff;

    using EntityId = uint64_t;

    inline uint64_t entityIndexFromEntityId(EntityId id)
    {
        return (id & EntityIdEntityMask);
    };

    inline uint64_t chunkIndexFromEntityId(EntityId id)
    {
        return (id & EntityIdChunkMask) >> 16;
    };

    inline ComponentArcheTypeId archeTypeIdFromEntityId(EntityId id)
    {
        return (id & EntityIdArcheTypeMask) >> 40;
    };

    inline EntityId createEntityId(
        uint64_t index,
        uint64_t chunkIndex,
        ComponentArcheTypeId archeType)
    {
        return (archeType << 40) | (chunkIndex << 16) | index;
    };

    class Ecs;
    class Entity
    {
    public:
        Entity(Ecs* ecs, TypeStorage* componentTypeStorage, EntityId _entityId)
            : m_ecs{ ecs }
            , m_componentTypeStorage{ componentTypeStorage }
            , entityId{ _entityId }
        {}

    private:
        Ecs* m_ecs;
        TypeStorage* m_componentTypeStorage;
    public:

        EntityId entityId;
        
        ComponentArcheTypeId archeType()
        {
            return archeTypeIdFromEntityId(entityId);
        }

        template<typename T>
        void addComponent()
        {
            addComponent(m_componentTypeStorage->typeId<typename std::remove_reference<T>::type>());
        }

        template<typename... T>
        typename std::enable_if<sizeof...(T) == 0>::type
            unpackTypes(ArcheTypeSet& result) { }

        template<typename T, typename... Rest>
        void unpackTypes(ArcheTypeSet& result)
        {
            result.set(m_componentTypeStorage->typeId<typename std::remove_reference<T>::type>());
            unpackTypes<Rest& ...>(result);
        }

        template<typename T, typename... Rest>
        void addComponents()
        {
            ArcheTypeSet typeIndexes;
            unpackTypes<T, Rest...>(typeIndexes);
            addComponents(typeIndexes);
        }

        template<typename T, typename... Rest>
        void setComponents()
        {
            ArcheTypeSet typeIndexes;
            unpackTypes<T, Rest...>(typeIndexes);
            setComponents(typeIndexes);
        }

        template<typename T>
        bool hasComponent()
        {
            return hasComponent(m_componentTypeStorage->typeId<typename std::remove_reference<T>::type>());
        }

        template<typename T>
        void removeComponent()
        {
            removeComponent(m_componentTypeStorage->typeId<typename std::remove_reference<T>::type>());
        }

        template<typename T>
        T& component()
        {
            T* ptr = reinterpret_cast<T*>(component(m_componentTypeStorage->typeId<typename std::remove_reference<T>::type>()));
            return *(ptr + entityIndexFromEntityId(entityId));
        }

        void addComponent(ComponentTypeId componentTypeId);
        void addComponents(const ArcheTypeSet& typeIndexes);
        void addComponents(const ArcheTypeSet& typeIndexes, ComponentArcheTypeId id);
        void setComponents(const ArcheTypeSet& typeIndexes);
        void setComponents(ComponentArcheTypeId id);
        bool hasComponent(ComponentTypeId componentTypeId);
        void removeComponent(ComponentTypeId componentTypeId);
        void* component(ComponentTypeId componentTypeId);

        bool hasComponents(const std::vector<ComponentTypeId>& componentIds)
        {
            for (auto&& id : componentIds)
                if (!hasComponent(id))
                    return false;
            return true;
        }

        bool operator==(const Entity& entity) { return entityId == entity.entityId; }
        bool operator!=(const Entity& entity) { return entityId != entity.entityId; }
    };
}
