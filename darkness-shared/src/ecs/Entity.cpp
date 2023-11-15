#include "ecs/Entity.h"
#include "ecs/Ecs.h"

namespace ecs
{
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

    void* Entity::component(ComponentTypeId componentTypeId)
    {
        return m_ecs->component(*this, componentTypeId);
    }
}
