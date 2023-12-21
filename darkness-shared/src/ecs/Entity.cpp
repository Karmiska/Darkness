#include "ecs/Entity.h"
#include "ecs/Ecs.h"

namespace ecs
{
    void Entity::addComponent(TypeId componentTypeId)
    {
        m_ecs->addComponent(*this, componentTypeId);
    }

    void Entity::addComponents(const ArcheTypeSet& typeIndexes)
    {
        m_ecs->addComponents(*this, typeIndexes);
    }

    void Entity::addComponents(const ArcheTypeSet& typeIndexes, ArcheTypeId id)
    {
        m_ecs->addComponents(*this, typeIndexes, id);
    }

    void Entity::setComponents(const ArcheTypeSet& typeIndexes)
    {
        m_ecs->setComponents(*this, typeIndexes);
    }

    void Entity::setComponents(ArcheTypeId id)
    {
        m_ecs->setComponents(*this, id);
    }

    bool Entity::hasComponent(TypeId componentTypeId)
    {
        return m_ecs->hasComponent(*this, componentTypeId);
    }

    void Entity::removeComponent(TypeId componentTypeId)
    {
        m_ecs->removeComponent(*this, componentTypeId);
    }

    void* Entity::component(TypeId componentTypeId)
    {
        return m_ecs->component(*this, componentTypeId);
    }
}
