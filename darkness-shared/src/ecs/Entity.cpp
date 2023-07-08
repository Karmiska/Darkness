#include "ecs/Entity.h"
#include "ecs/Ecs.h"

namespace ecs
{
    uint64_t entityIndexFromEntityId(EntityId id)
    {
        return (id & EntityIdEntityMask);
    };

    uint64_t chunkIndexFromEntityId(EntityId id)
    {
        return (id & EntityIdChunkMask) >> 16;
    };

    ComponentArcheTypeId archeTypeIdFromEntityId(EntityId id)
    {
        return (id & EntityIdArcheTypeMask) >> 40;
    };

    EntityId createEntityId(
        uint64_t index,
        uint64_t chunkIndex,
        ComponentArcheTypeId archeType)
    {
        return (archeType << 40) | (chunkIndex << 16) | index;
    };

    void Entity::addComponent(ComponentTypeId componentTypeId)
    {
        m_ecs->addComponent(*this, componentTypeId);
    }

    bool Entity::hasComponent(ComponentTypeId componentTypeId)
    {
        return m_ecs->hasComponent(*this, componentTypeId);
    }

    void Entity::removeComponent(ComponentTypeId componentTypeId)
    {
        m_ecs->removeComponent(*this, componentTypeId);
    }
}
