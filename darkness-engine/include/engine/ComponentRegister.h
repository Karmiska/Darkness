#pragma once

#include "containers/string.h"
#include "containers/unordered_map.h"
#include "containers/string.h"
#include "containers/vector.h"
#include <functional>
#include "containers/memory.h"

namespace engine
{
    class Property;
    class ComponentRegister
    {
    public:
        virtual ~ComponentRegister() {};
        int propertyCount() const;
        engine::vector<engine::string> propertyNames() const;

        bool hasVariant(const engine::string& propertyName) const;
        const Property& variant(const engine::string& propertyName) const;
        Property& variant(const engine::string& propertyName);

        template<typename T>
        void insertLoadedValue(const engine::string& propertyName, T value)
        {
            m_loadedButNotExistingProperties[propertyName] = engine::make_shared<Property>(nullptr, "temporary", value);
        }

        void registerProperty(const engine::string& propertyName, Property* variant);
        void unregisterProperty(const engine::string& propertyName, Property* variant);

        void registerForChanges(void* client, std::function<void()> change)
        {
            m_changeNotifications[client] = change;
        }
        void unregisterForChanges(void* client)
        {
            m_changeNotifications.erase(client);
        }

    protected:
        mutable engine::unordered_map<engine::string, Property*> m_properties;
        mutable engine::unordered_map<engine::string, engine::shared_ptr<Property>> m_loadedButNotExistingProperties;
        engine::unordered_map<void*, std::function<void()>> m_changeNotifications;
    };
}
