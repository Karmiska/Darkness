#include "engine/Ecs.h"

namespace engine
{
#if 0
    Ecs::Ecs()
    {
        m_entities.reserve(100000000);
    }

    Entity Ecs::createEntity()
    {
        auto entityId = m_entities.size();
        m_entities.emplace_back(EntityStorage{ *this });
        return Entity{ this, entityId };
    };
#endif
}
