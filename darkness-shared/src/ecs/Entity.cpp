#include "ecs/Entity.h"
#include "ecs/Ecs.h"

namespace ecs
{
    void Entity::addComponent(ComponentTypeId componentTypeId)
    {
        m_ecs->addComponent(*this, componentTypeId);
    }

    void Entity::addComponents(const ArcheTypeSet& typeIndexes)
    {
        m_ecs->addComponents(*this, typeIndexes);
    }

    void Entity::addComponents(const ArcheTypeSet& typeIndexes, ComponentArcheTypeId id)
    {
        m_ecs->addComponents(*this, typeIndexes, id);
    }

    void Entity::setComponents(const ArcheTypeSet& typeIndexes)
    {
        m_ecs->setComponents(*this, typeIndexes);
    }

    void Entity::setComponents(ComponentArcheTypeId id)
    {
        m_ecs->setComponents(*this, id);
    }

    bool Entity::hasComponent(ComponentTypeId componentTypeId)
    {
        return m_ecs->hasComponent(*this, componentTypeId);
    }

    void Entity::removeComponent(ComponentTypeId componentTypeId)
    {
        m_ecs->removeComponent(*this, componentTypeId);
    }

    void* Entity::component(ComponentTypeId componentTypeId)
    {
        return m_ecs->component(*this, componentTypeId);
    }
}
