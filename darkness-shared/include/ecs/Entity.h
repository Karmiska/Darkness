#pragma once

#include "ecs/ComponentTypeStorage.h"
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
        Entity(Ecs* ecs, EntityId _entityId)
            : m_ecs{ ecs }
            , entityId{ _entityId }
        {}

    private:
        Ecs* m_ecs;
    public:

        EntityId entityId;
        
        ComponentArcheTypeId archeType()
        {
            return archeTypeIdFromEntityId(entityId);
        }

        template<typename T>
        void addComponent()
        {
            addComponent(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>());
        }

        template<typename... T>
        typename std::enable_if<sizeof...(T) == 0>::type
            unpackTypes(ArcheTypeSet& result) { }

        template<typename T, typename... Rest>
        void unpackTypes(ArcheTypeSet& result)
        {
            result.set(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>());
            unpackTypes<Rest& ...>(result);
        }

        template<typename T, typename... Rest>
        void addComponents()
        {
            ArcheTypeSet typeIndexes;
            unpackTypes<T, Rest...>(typeIndexes);
            addComponents(typeIndexes);
        }

        template<typename T>
        bool hasComponent()
        {
            return hasComponent(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>());
        }

        template<typename T>
        void removeComponent()
        {
            removeComponent(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>());
        }

        template<typename T>
        T& component()
        {
            T* ptr = reinterpret_cast<T*>(component(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>()));
            return *(ptr + entityIndexFromEntityId(entityId));
        }

        void addComponent(ComponentTypeId componentTypeId);
        void addComponents(const ArcheTypeSet& typeIndexes);
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
