#pragma once

#include "containers/vector.h"
#include <functional>
#include "containers/memory.h"
#include "DrawerCallbacks.h"

class Visual;
struct DrawerCallbacks;

class Property
{
public:
    Property() {};

    Property(const Property& prop)
        : m_impl{ prop.m_impl }
        , m_name{ prop.m_name }
        , createVariantDrawer{ prop.createVariantDrawer }
    {
    }

    Property& operator=(const Property& prop) = delete;
    Property(Property&& prop)
        : m_impl{ std::move(prop.m_impl) }
        , m_name{ std::move(prop.m_name) }
        , createVariantDrawer{ std::move(prop.createVariantDrawer) }
    {}
    Property& operator=(Property&&) = delete;

    template <class T>
    Property(/*EngineComponent* component, */const std::string& name, T _value)
        : m_impl(engine::make_shared<VariantImpl<typename std::decay<T>::type>>(_value))
        , m_name{ name }
    {

        createVariantDrawer = [&](DrawerCallbacks callback)->engine::shared_ptr<Visual>
        {
            return createDrawer<typename std::decay<T>::type>
                (dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get()).m_value, callback);
        };
        /*writeJson = [&](rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
        {
            writeJsonValue<typename std::decay<T>::type>(writer, m_name, dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get()).m_value);
        };
        component->registerProperty(name, this);*/
    }

    template <class T>
    T& value()
    {
        return dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get()).m_value;
    }

    template <class T>
    const T& value() const
    {
        return dynamic_cast<VariantImpl<typename std::decay<T>::type>&>(*m_impl.get()).m_value;
    }

    template <class T>
    void value(const T& _value)
    {
        m_impl = engine::make_shared<VariantImpl<typename std::decay<T>::type>>(_value);
    }

    const std::string& name() const
    {
        return m_name;
    }

    
    //std::function<void(rapidjson::PrettyWriter<rapidjson::StringBuffer>&)> writeJson;

private:
	struct AbstractVariantImpl
	{
		virtual ~AbstractVariantImpl() {}
	};

	template <class T>
	struct VariantImpl : public AbstractVariantImpl
	{
		VariantImpl(T value)
			: m_value{ value }
		{}

		T m_value;
	};
	
	engine::shared_ptr<AbstractVariantImpl> m_impl;
    std::string m_name;
public:
	std::function<engine::shared_ptr<Visual>(DrawerCallbacks)> createVariantDrawer;
    
};
