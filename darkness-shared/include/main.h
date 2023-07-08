#pragma once

#include "Common.h"
#include "engine/primitives/Rect.h"
#include "containers/string.h"
#include "containers/memory.h"

void forceFunctionInclude();

struct DrawerCallbacks;
class Visual;

struct DrawerHolder
{
    DrawerCallbacks* callbacks;
    engine::shared_ptr<Visual> data;
};

namespace serialization
{
    class TypeRegistry;
    class Stream;
}

struct TypeInstance
{
    serialization::TypeRegistry* registry;
    uint32_t instanceRegistryIndex;
    void* instance;
};

extern "C"
{
    DLLEXPORT serialization::TypeRegistry* CALLCONV createTypeRegistry();
    DLLEXPORT void CALLCONV destroyTypeRegistry(serialization::TypeRegistry*);


    DLLEXPORT uint32_t CALLCONV numberOfTypes(serialization::TypeRegistry* typeRegistry);
    DLLEXPORT void CALLCONV typeName(serialization::TypeRegistry* typeRegistry, uint32_t index, char* buffer, uint32_t sizeBytes);
    DLLEXPORT TypeInstance* CALLCONV createTypeInstance(serialization::TypeRegistry* typeRegistry, uint32_t index);
    DLLEXPORT void CALLCONV destroyTypeInstance(TypeInstance* instance);


    DLLEXPORT uint32_t CALLCONV numberOfProperties(TypeInstance* instance);
    DLLEXPORT void CALLCONV propertyName(TypeInstance* instance, uint32_t propertyIndex, char* buffer, uint32_t sizeBytes);
    typedef void (*CBDrawText)(void* propertyWidget, engine::Rect, const engine::string&);


    DLLEXPORT DrawerCallbacks* CALLCONV createDrawerCallbacks(void* propertyWidget, CBDrawText drawText);
    DLLEXPORT void CALLCONV destroyDrawerCallbacks(DrawerCallbacks* drawerCallbacks);
    DLLEXPORT DrawerHolder* CALLCONV createPropertyDrawer(TypeInstance* instance, uint32_t propertyIndex, DrawerCallbacks* drawerCallbacks);
    DLLEXPORT void CALLCONV destroyPropertyDrawer(DrawerHolder* drawer);
    DLLEXPORT void CALLCONV drawPropertyDrawer(DrawerHolder* drawer, int x, int y, int width, int height);


    DLLEXPORT serialization::Stream* CALLCONV serializeType(TypeInstance* instance);
    DLLEXPORT uint32_t CALLCONV serializeDataSize(serialization::Stream* data);
    DLLEXPORT void CALLCONV serializeFetch(serialization::Stream* data, void* buffer);
    DLLEXPORT void CALLCONV deserializeType(TypeInstance* instance, void* buffer, uint32_t sizeBytes);
}
