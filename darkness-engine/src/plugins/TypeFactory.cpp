#include "plugins/TypeFactory.h"
#include <algorithm>

namespace engine
{
    void TypeFactory::registerType(
        const engine::string& typeName,
        CreateFunction create,
        DestroyFunction destroy)
    {
        if (m_types.find(typeName) == m_types.end())
        {
            m_types[typeName] = TypeContainer{ typeName, create, destroy };
        }
    }

    void TypeFactory::unregisterType(
        const engine::string& typeName)
    {
        auto container = m_types.find(typeName);
        if(container != m_types.end())
        {
            m_types.erase(container);
        }
    }

    const engine::unordered_map<engine::string, TypeContainer>& TypeFactory::types() const
    {
        return m_types;
    }

    void* TypeFactory::createType(const engine::string& typeName) const
    {
        auto container = m_types.find(typeName);
        if (container != m_types.end())
        {
            return container->second.create();
        }
        return nullptr;
    }

    void TypeFactory::destroyType(const engine::string& typeName, void* typeInstance) const
    {
        auto container = m_types.find(typeName);
        if (container != m_types.end())
        {
            return container->second.destroy(typeInstance);
        }
    }
}
