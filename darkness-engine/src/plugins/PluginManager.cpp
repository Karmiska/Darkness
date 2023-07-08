#include "plugins/PluginManager.h"
#include "platform/Directory.h"
#include "tools/Debug.h"

namespace engine
{
#ifdef _WIN32
    void PluginManager::addFolder(const engine::string& folder)
    {
#if 1
        Directory directory(folder);
        for (auto file : directory.files())
        {
            engine::string libPath = folder + file;

            size_t marker = libPath.find_last_of('.') + 1;
            engine::string fileExtension = libPath.substr(marker, libPath.length() - marker);

            if (fileExtension == "plugin")
            {
                SetLastError(0);
                HINSTANCE instance = LoadLibraryA(libPath.data());
                auto err = GetLastError();
                if (!instance && err != 0)
                {
#ifndef _DURANGO
                    LPSTR messageBuffer = nullptr;
                    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                    engine::string message(messageBuffer, size);
                    LocalFree(messageBuffer);
#endif
                }
                else
                {
                    m_instances.emplace_back(PluginInstance{ instance });
                }
            }
        }
#endif
    }

    TypeFactory& PluginManager::factory()
    {
        return m_typeFactory;
    }

    PluginManager::PluginInstance::PluginInstance(HINSTANCE _instance)
        : instance{ _instance }
        /*, m_createTypeRegistry{ nullptr }
        , m_destroyTypeRegistry{ nullptr }
        , m_initializeTypes{ nullptr }
        , m_numberOfTypes{ nullptr }
        , m_typeName{ nullptr }
        , m_createTypeInstance{ nullptr }
        , m_destroyTypeInstance{ nullptr }
        , m_numberOfProperties{ nullptr }
        , m_propertyName{ nullptr }

        , m_createDrawerCallbacks{ nullptr }
        , m_destroyDrawerCallbacks{ nullptr }

        , m_createPropertyDrawer{ nullptr }
        , m_destroyPropertyDrawer{ nullptr }
        , m_drawPropertyDrawer{ nullptr }

        , m_serializeType{ nullptr }
        , m_serializeDataSize{ nullptr }
        , m_serializeFetch{ nullptr }
        , m_deserializeType{ nullptr }*/
    {
        m_apiPointers.reset(new ApiPointers(), [&](ApiPointers* ptrs) {
            if (ptrs->registry && ptrs->m_destroyTypeRegistry)
                ptrs->m_destroyTypeRegistry(ptrs->registry);
        });

        m_apiPointers->m_createTypeRegistry = (CreateTypeRegistry)GetProcAddress(instance, "createTypeRegistry");
        m_apiPointers->m_destroyTypeRegistry = (DestroyTypeRegistry)GetProcAddress(instance, "destroyTypeRegistry");
        m_apiPointers->m_initializeTypes = (InitializeTypes)GetProcAddress(instance, "initializeTypes");
        
        m_apiPointers->m_numberOfTypes = (NumberOfTypes)GetProcAddress(instance, "numberOfTypes");
        m_apiPointers->m_typeName = (TypeName)GetProcAddress(instance, "typeName");
        m_apiPointers->m_createTypeInstance = (CreateTypeInstance)GetProcAddress(instance, "createTypeInstance");
        m_apiPointers->m_destroyTypeInstance = (DestroyTypeInstance)GetProcAddress(instance, "destroyTypeInstance");
        m_apiPointers->m_numberOfProperties = (NumberOfProperties)GetProcAddress(instance, "numberOfProperties");
        m_apiPointers->m_propertyName = (PropertyName)GetProcAddress(instance, "propertyName");
        
        m_apiPointers->m_createDrawerCallbacks = (CreateDrawerCallbacks)GetProcAddress(instance, "createDrawerCallbacks");
        m_apiPointers->m_destroyDrawerCallbacks = (DestroyDrawerCallbacks)GetProcAddress(instance, "destroyDrawerCallbacks");
        
        m_apiPointers->m_createPropertyDrawer = (CreatePropertyDrawer)GetProcAddress(instance, "createPropertyDrawer");
        m_apiPointers->m_destroyPropertyDrawer = (DestroyPropertyDrawer)GetProcAddress(instance, "destroyPropertyDrawer");
        m_apiPointers->m_drawPropertyDrawer = (DrawPropertyDrawer)GetProcAddress(instance, "drawPropertyDrawer");
        
        m_apiPointers->m_serializeType = (SerializeType)GetProcAddress(instance, "serializeType");
        m_apiPointers->m_serializeDataSize = (SerializeDataSize)GetProcAddress(instance, "serializeDataSize");
        m_apiPointers->m_serializeFetch = (SerializeFetch)GetProcAddress(instance, "serializeFetch");
        m_apiPointers->m_deserializeType = (DeserializeType)GetProcAddress(instance, "deserializeType");

        ASSERT(
            m_apiPointers->m_createTypeRegistry &&
            m_apiPointers->m_destroyTypeRegistry &&
            m_apiPointers->m_initializeTypes);

        ASSERT(
            m_apiPointers->m_numberOfTypes && 
            m_apiPointers->m_typeName && 
            m_apiPointers->m_createTypeInstance && 
            m_apiPointers->m_destroyTypeInstance && 
            m_apiPointers->m_numberOfProperties && 
            m_apiPointers->m_propertyName);

        ASSERT(
            m_apiPointers->m_createDrawerCallbacks && 
            m_apiPointers->m_destroyDrawerCallbacks);

        ASSERT(
            m_apiPointers->m_createPropertyDrawer && 
            m_apiPointers->m_destroyPropertyDrawer && 
            m_apiPointers->m_drawPropertyDrawer);

        ASSERT(
            m_apiPointers->m_serializeType && 
            m_apiPointers->m_serializeDataSize && 
            m_apiPointers->m_serializeFetch && 
            m_apiPointers->m_deserializeType);

        m_apiPointers->registry = m_apiPointers->m_createTypeRegistry();
        m_apiPointers->m_initializeTypes(m_apiPointers->registry);

        uint32_t typeCount = m_apiPointers->m_numberOfTypes(m_apiPointers->registry);
        for (uint32_t i = 0; i < typeCount; ++i)
        {
            m_pluginTypes.emplace_back(engine::make_shared<PluginType>( m_apiPointers, i ));
        }
    }
#else
    void PluginManager::addFolder(engine::string folder)
    {
    }
    
    TypeFactory& PluginManager::factory()
    {
        return m_typeFactory;
    }
    
    PluginManager::PluginInstance::PluginInstance(InstanceHandle _instance)
    : instance{ _instance }
    {
        m_apiPointers.reset(new ApiPointers(), [&](ApiPointers* ptrs) {
            if (ptrs->registry && ptrs->m_destroyTypeRegistry)
                ptrs->m_destroyTypeRegistry(ptrs->registry);
        });
        
    }
    
#endif
}
