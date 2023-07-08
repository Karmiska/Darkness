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

    uint64_t entityIndexFromEntityId(EntityId id);
    uint64_t chunkIndexFromEntityId(EntityId id);
    ComponentArcheTypeId archeTypeIdFromEntityId(EntityId id);
    EntityId createEntityId(
        uint64_t index,
        uint64_t chunkIndex,
        ComponentArcheTypeId archeType);

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

        void addComponent(ComponentTypeId componentTypeId);
        bool hasComponent(ComponentTypeId componentTypeId);
        void removeComponent(ComponentTypeId componentTypeId);

        bool hasComponents(const std::vector<ComponentTypeId>& componentIds)
        {
            for (auto&& id : componentIds)
                if (!hasComponent(id))
                    return false;
            return true;
        }
    };
}
