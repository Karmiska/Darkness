#pragma once

#include "plugins/TypeFactory.h"
#include "tools/Debug.h"
#include "containers/string.h"
#include <map>
#include "containers/vector.h"
#include "containers/memory.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#define CALLCONV
#endif

namespace engine
{
    struct Rect
    {
        int x;
        int y;
        int width;
        int height;
    };

    typedef void(*CBDrawText)(void*, Rect, const engine::string&);

    struct PluginDrawer
    {
        void* callbacks;
        void* drawer;
    };

    typedef void*( *CreateTypeRegistry)();
    typedef void( *DestroyTypeRegistry)(void*);
    typedef void( *InitializeTypes)(void*);
    typedef uint32_t( *NumberOfTypes)(void*);
    typedef void( *TypeName)(void*, uint32_t, char*, uint32_t);
    typedef void*( *CreateTypeInstance)(void*, uint32_t);
    typedef void( *DestroyTypeInstance)(void*);
    typedef uint32_t( *NumberOfProperties)(void*);
    typedef void( *PropertyName)(void*, uint32_t, char*, uint32_t);

    typedef void*( *CreateDrawerCallbacks)(void*, CBDrawText);
    typedef void( *DestroyDrawerCallbacks)(void*);

    typedef void*( *CreatePropertyDrawer)(void*, uint32_t, void*);
    typedef void( *DestroyPropertyDrawer)(void*);
    typedef void( *DrawPropertyDrawer)(void*, int, int, int, int);

    typedef void*( *SerializeType)(void*);
    typedef uint32_t( *SerializeDataSize)(void*);
    typedef void( *SerializeFetch)(void*, void*);
    typedef void( *DeserializeType)(void*, void*, uint32_t);

    struct ApiPointers
    {
        CreateTypeRegistry m_createTypeRegistry;
        DestroyTypeRegistry m_destroyTypeRegistry;
        InitializeTypes m_initializeTypes;

        NumberOfTypes m_numberOfTypes;
        TypeName m_typeName;
        CreateTypeInstance m_createTypeInstance;
        DestroyTypeInstance m_destroyTypeInstance;
        NumberOfProperties m_numberOfProperties;
        PropertyName m_propertyName;

        CreateDrawerCallbacks m_createDrawerCallbacks;
        DestroyDrawerCallbacks m_destroyDrawerCallbacks;

        CreatePropertyDrawer m_createPropertyDrawer;
        DestroyPropertyDrawer m_destroyPropertyDrawer;
        DrawPropertyDrawer m_drawPropertyDrawer;

        SerializeType m_serializeType;
        SerializeDataSize m_serializeDataSize;
        SerializeFetch m_serializeFetch;
        DeserializeType m_deserializeType;

        void* registry;
    };

    class PluginProperty
    {
    public:
        PluginProperty(
            engine::shared_ptr<ApiPointers> apiPointers,
            void* typeInstance,
            uint32_t propertyIndex)
            : m_apiPointers{ apiPointers }
            , m_typeInstance{ typeInstance }
            , m_propertyIndex{ propertyIndex }
        {
            char typeNameBuffer[1024] = { 0 };
            m_apiPointers->m_propertyName(m_typeInstance, propertyIndex, &typeNameBuffer[0], 1024);
            m_name = typeNameBuffer;
        }

        const engine::string& name() const
        {
            return m_name;
        }

        PluginDrawer* createDrawer(void* propertyWidget, CBDrawText cbDrawText)
        {
            PluginDrawer* res = new PluginDrawer();
            res->callbacks = m_apiPointers->m_createDrawerCallbacks(propertyWidget, cbDrawText);
            res->drawer = m_apiPointers->m_createPropertyDrawer(m_typeInstance, m_propertyIndex, res->callbacks);
            return res;
        }
        void destroyDrawer(PluginDrawer* ptr)
        {
            m_apiPointers->m_destroyPropertyDrawer(ptr->drawer);
            m_apiPointers->m_destroyDrawerCallbacks(ptr->callbacks);
            delete ptr;
        }

        void callDraw(PluginDrawer* drawer, Rect rect)
        {
            m_apiPointers->m_drawPropertyDrawer(drawer->drawer, rect.x, rect.y, rect.width, rect.height);
        }

        template <typename T>
        void value(T value)
        {

        }

        template <typename T>
        T value()
        {
            return{};
        }

    private:
        engine::shared_ptr<ApiPointers> m_apiPointers;
        void* m_typeInstance;
        uint32_t m_propertyIndex;
        engine::string m_name;
    };

    class PluginType
    {
    public:
        PluginType(
            engine::shared_ptr<ApiPointers> apiPointers,
            uint32_t typeIndex)
            : m_apiPointers{ apiPointers }
            , m_typeIndex{ typeIndex }
        {
            char nameBuffer[1024] = { 0 };
            m_apiPointers->m_typeName(m_apiPointers->registry, typeIndex, &nameBuffer[0], 1024);
            m_name = nameBuffer;
        }
        engine::string& name() { return m_name; }
        const engine::string& name() const { return m_name; }

        void* createInstance()
        {
            return m_apiPointers->m_createTypeInstance(m_apiPointers->registry, m_typeIndex);
        }

        void destroyInstance(void* ptr)
        {
            m_apiPointers->m_destroyTypeInstance(ptr);
        }

        engine::vector<PluginProperty> properties(void* typeInstance)
        {
            engine::vector<PluginProperty> res;
            auto propertyCount = m_apiPointers->m_numberOfProperties(typeInstance);
            for (uint32_t a = 0; a < propertyCount; ++a)
            {
                res.emplace_back(m_apiPointers, typeInstance, a);
            }
            return res;
        }

    private:
        engine::string m_name;
        engine::shared_ptr<ApiPointers> m_apiPointers;
        uint32_t m_typeIndex;
    };

    class TypeInstance
    {
    public:
        TypeInstance()
            : m_instance{ nullptr }
            , m_name{ "" }
        {}

        TypeInstance(
            engine::shared_ptr<void> instance, 
            const engine::string& name,
            PluginType* typePtr)
            : m_instance{ instance }
            , m_name{ name }
            , m_typePtr{ typePtr }
        {}

        const engine::string& name() const
        {
            return m_name;
        }

        engine::vector<PluginProperty> properties()
        {
            return m_typePtr->properties(m_instance.get());
        }

        /*PluginProperty& property(const engine::string& name)
        {
            return {};
        }*/

    private:
        engine::shared_ptr<void> m_instance;
        engine::string m_name;
        PluginType* m_typePtr;
    };

#ifdef _WIN32
#define InstanceHandle HINSTANCE
#else
#define InstanceHandle void*
#endif
    
    class PluginManager
    {
    public:
        void addFolder(const engine::string& folder);
        TypeFactory& factory();

        TypeInstance createType(const engine::string& typeName)
        {
            for (auto&& pluginInstance : m_instances)
            {
                for (auto&& typeInstance : pluginInstance.types())
                {
                    if (typeInstanceNameEquals(typeInstance->name(), typeName))
                    {
                        engine::shared_ptr<void> sptr = engine::shared_ptr<void>(typeInstance->createInstance(),
                            [&](void* ptr) {
                            typeInstance->destroyInstance(ptr);
                        });

                        TypeInstance res(sptr, typeName, typeInstance.get());
                        return res;
                    }
                }
            }
            ASSERT(true);
            return TypeInstance();
        }

    private:

        class PluginInstance
        {
        public:
            PluginInstance(InstanceHandle _instance);

            engine::vector<engine::shared_ptr<PluginType>>& types() { return m_pluginTypes; }
        private:
            InstanceHandle instance;

            engine::shared_ptr<ApiPointers> m_apiPointers;
            engine::vector<engine::shared_ptr<PluginType>> m_pluginTypes;
        };

        bool typeInstanceNameEquals(const engine::string& generatedName, const engine::string& name)
        {
            return (generatedName.find("class") != engine::string::npos) && (generatedName.find(name) != engine::string::npos);
        }

        TypeFactory m_typeFactory;
        engine::vector<PluginInstance> m_instances;
    };
}
