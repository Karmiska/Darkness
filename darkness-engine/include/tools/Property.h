#pragma once

#include "engine/Drawer.h"
#include "engine/EngineComponent.h"
#include "tools/Serialization.h"
#include "containers/unordered_map.h"
#include "containers/memory.h"
#include "containers/string.h"
#include <functional>

namespace engine
{

    using PropertyChanged = std::function<void()>;

    class Property
    {
    public:
        Property() {};

        Property(const Property&) = delete;
        Property& operator=(const Property&) = delete;
        Property(Property&&) = delete;
        Property& operator=(Property&&) = delete;

        template <class T>
        Property(EngineComponent* component, const engine::string& name, T _value, PropertyChanged onChanged)
            : m_impl(engine::make_shared<VariantImpl<typename std::decay<T>::type>>(_value))
            , m_name{ name }
            , m_component{ component }
        {
            m_onChanged[this] = { onChanged };
#ifndef GAME_BUILD
            createVariantDrawer = [&]()->engine::shared_ptr<Drawer>
            {
                return createDrawer<typename std::decay<T>::type>(*this);
            };
#endif
            writeJson = [&](rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
            {
                writeJsonValue<typename std::decay<T>::type>(writer, m_name, dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get()).m_value);
            };
            if(component)
                component->registerProperty(name, this);
        }

        void swap(Property& property)
        {
            std::swap(m_impl, property.m_impl);
        }

        template <class T>
        Property(EngineComponent* component, const engine::string& name, T _value)
            : m_impl(engine::make_shared<VariantImpl<typename std::decay<T>::type>>(_value))
            , m_name{ name }
            , m_component{ component }
        {
#ifndef GAME_BUILD
            createVariantDrawer = [&]()->engine::shared_ptr<Drawer>
            {
                return createDrawer<typename std::decay<T>::type>(*this);
            };
#endif
            writeJson = [&](rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
            {
                writeJsonValue<typename std::decay<T>::type>(writer, m_name, dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get()).m_value);
            };
            if(component)
                component->registerProperty(name, this);
        }

        ~Property()
        {
            for (auto&& client : m_onRemove)
            {
                for (auto&& onRemoved : client.second)
                {
                    onRemoved();
                }
            }

            if(m_component)
                m_component->unregisterProperty(m_name, this);
        }

        template<typename T>
        void setRangeCheck(std::function<bool(T, T&)> check)
        {
            auto im = dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get());
            im.m_rangeCheck = check;
        }

        template<typename T>
        bool rangeCheck(T value, T& closestPossible)
        {
            auto im = dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get());
            if (im.m_rangeCheck)
                return im.m_rangeCheck(value, closestPossible);
            closestPossible = value;
            return true;
        }

        void registerForRemovalNotification(void* ptr, PropertyChanged onRemove)
        {
            m_onRemove[ptr].emplace_back(onRemove);
        }

        void unregisterForRemovalNotification(void* ptr)
        {
            int occurance = 0;
            for (auto&& keyValue : m_onRemove)
            {
                if (keyValue.first == ptr)
                    ++occurance;
            }
            ASSERT(occurance == 1, "Invalid register count");
            m_onChanged.erase(ptr);
        }

        void registerForChangeNotification(void* ptr, PropertyChanged onChanged)
        {
            m_onChanged[ptr].emplace_back(onChanged);
        }

        void unregisterForChangeNotification(void* ptr)
        {
            int occurance = 0;
            for (auto&& keyValue : m_onChanged)
            {
                if (keyValue.first == ptr)
                    ++occurance;
            }
            ASSERT(occurance == 1, "Invalid register count");
            m_onChanged.erase(ptr);
        }

        template <class T>
        const T& value() const
        {
            return dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get()).m_value;
        }

        template <class T>
        void value(const T& _value)
        {
            auto im = dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get());
            T closest;
            if (im.m_rangeCheck && im.m_rangeCheck(_value, closest))
            {
                m_impl = engine::make_shared<VariantImpl<typename std::decay<T>::type>>(closest);
            }
            else
            {
                m_impl = engine::make_shared<VariantImpl<typename std::decay<T>::type>>(_value);
            }

            if(m_component)
                m_component->onValueChanged();
            for (auto&& client : m_onChanged)
            {
                for (auto&& onChanged : client.second)
                {
                    onChanged();
                }
            }
        }

        const engine::string& name() const
        {
            return m_name;
        }

#ifndef GAME_BUILD
        std::function<engine::shared_ptr<Drawer>()> createVariantDrawer;
#endif
        std::function<void(rapidjson::PrettyWriter<rapidjson::StringBuffer>&)> writeJson;

    private:
        engine::unordered_map<void*, engine::vector<PropertyChanged>> m_onChanged;
        engine::unordered_map<void*, engine::vector<PropertyChanged>> m_onRemove;

        struct AbstractVariantImpl { virtual ~AbstractVariantImpl() {} };

        template <class T>
        struct VariantImpl : public AbstractVariantImpl
        {
            VariantImpl(T value)
                : m_value{ value }
            {}

            T m_value;
            std::function<bool(T, T&)> m_rangeCheck;
        };

        engine::shared_ptr<AbstractVariantImpl> m_impl;

        engine::string m_name;
        EngineComponent* m_component;
    };
}
