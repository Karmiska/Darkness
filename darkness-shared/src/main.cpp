#include "main.h"
#include "Common.h"
#include "TypeRegistry.h"

using namespace serialization;
using namespace engine;

void drawText(void* /*propertyWidget*/, Rect /*rect*/, const engine::string& /*str*/)
{}

void forceFunctionInclude()
{
    auto drawerCallbacks = createDrawerCallbacks(nullptr, drawText);

    auto registry = createTypeRegistry();
    uint32_t numTypes = numberOfTypes(registry);
    for (int i = 0; i < static_cast<int>(numTypes); ++i)
    {
        char buffer[1024] = { 0 };
        typeName(registry, i, &buffer[0], 1024);

        auto typeInstance = createTypeInstance(registry, i);

        auto numProperties = numberOfProperties(typeInstance);
        for (int a = 0; a < static_cast<int>(numProperties); ++a)
        {
            char nameBuffer[1024] = { 0 };
            propertyName(typeInstance, a, &nameBuffer[0], 1024);

            auto typeDrawer = createPropertyDrawer(typeInstance, a, drawerCallbacks);
            drawPropertyDrawer(typeDrawer, 0, 0, 100, 100);
            destroyPropertyDrawer(typeDrawer);
        }

        auto serializedData = serializeType(typeInstance);
        uint32_t dataSize = serializeDataSize(serializedData);
        engine::vector<char> data(dataSize, 0);
        serializeFetch(serializedData, &data[0]);

        deserializeType(typeInstance, &data[0], static_cast<uint32_t>(data.size()));
        destroyTypeInstance(typeInstance);
    }
    destroyTypeRegistry(registry);
    destroyDrawerCallbacks(drawerCallbacks);
}

extern "C"
{
    DLLEXPORT TypeRegistry* CALLCONV createTypeRegistry()
    {
        return new TypeRegistry();
    }

    DLLEXPORT void CALLCONV destroyTypeRegistry(TypeRegistry* ptr)
    {
        delete ptr;
    }

    DLLEXPORT uint32_t CALLCONV numberOfTypes(TypeRegistry* typeRegistry)
    {
        return static_cast<uint32_t>(typeRegistry->types().size());
    }

    DLLEXPORT void CALLCONV typeName(TypeRegistry* typeRegistry, uint32_t index, char* buffer, uint32_t sizeBytes)
    {
        uint32_t nameLen = static_cast<uint32_t>(typeRegistry->types()[index].typeId().length());
        uint32_t bytesToCopy = nameLen < sizeBytes ? nameLen : sizeBytes;
        memcpy(buffer, typeRegistry->types()[index].typeId().data(), bytesToCopy);
    }

    DLLEXPORT TypeInstance* CALLCONV createTypeInstance(TypeRegistry* typeRegistry, uint32_t index)
    {
        TypeInstance* typeInstance = new TypeInstance();
        typeInstance->registry = typeRegistry;
        typeInstance->instanceRegistryIndex = index;
        typeInstance->instance = typeRegistry->newType(typeRegistry->types()[index].typeId());
        
        return typeInstance;
    }

    DLLEXPORT void CALLCONV destroyTypeInstance(TypeInstance* instance)
    {
        instance->registry->deleteType(instance->registry->types()[instance->instanceRegistryIndex].typeId(), instance->instance);
        delete instance;
    }

    DLLEXPORT uint32_t CALLCONV numberOfProperties(TypeInstance* instance)
    {
        return static_cast<uint32_t>(instance->registry->types()[instance->instanceRegistryIndex].properties().size());
    }

    DLLEXPORT void CALLCONV propertyName(TypeInstance* instance, uint32_t propertyIndex, char* buffer, uint32_t sizeBytes)
    {
        uint32_t nameLen = static_cast<uint32_t>(instance->registry->types()[instance->instanceRegistryIndex].properties()[propertyIndex]->name().length());
        uint32_t bytesToCopy = nameLen < sizeBytes ? nameLen : sizeBytes;
        memcpy(buffer, instance->registry->types()[instance->instanceRegistryIndex].properties()[propertyIndex]->name().data(), bytesToCopy);
    }

    DLLEXPORT DrawerCallbacks* CALLCONV createDrawerCallbacks(
        void* propertyWidget,
        CBDrawText drawText)
    {
        DrawerCallbacks* callbacks = new DrawerCallbacks();
        callbacks->drawString = drawText;
        callbacks->propertyWidget = propertyWidget;
        return callbacks;
    }

    DLLEXPORT void CALLCONV destroyDrawerCallbacks(DrawerCallbacks* drawerCallbacks)
    {
        delete drawerCallbacks;
    }

    DLLEXPORT DrawerHolder* CALLCONV createPropertyDrawer(
        TypeInstance* instance,
        uint32_t propertyIndex,
        DrawerCallbacks* drawerCallbacks)
    {
        DrawerHolder* holder = new DrawerHolder();
        holder->callbacks = drawerCallbacks;
        holder->data = instance->registry->types()[instance->instanceRegistryIndex].properties()[propertyIndex]->createVariantDrawer(*holder->callbacks);
        return holder;
    }

    DLLEXPORT void CALLCONV destroyPropertyDrawer(DrawerHolder* drawer)
    {
        delete drawer;
    }

    DLLEXPORT void CALLCONV drawPropertyDrawer(DrawerHolder* drawer, int x, int y, int width, int height)
    {
        drawer->data->draw({x, y, width, height});
    }

    DLLEXPORT Stream* CALLCONV serializeType(TypeInstance* instance)
    {
        serialization::Stream* stream = new serialization::Stream();
        instance->registry->serialize(instance->registry->types()[instance->instanceRegistryIndex].typeId(), *stream, instance->instance);
        return stream;
    }

    DLLEXPORT uint32_t CALLCONV serializeDataSize(Stream* data)
    {
        return static_cast<uint32_t>(data->size());
    }

    DLLEXPORT void CALLCONV serializeFetch(Stream* data, void* buffer)
    {
        memcpy(buffer, data->data(), data->size());
        delete data;
    }

    DLLEXPORT void CALLCONV deserializeType(TypeInstance* instance, void* buffer, uint32_t sizeBytes)
    {
        serialization::Stream stream(buffer, sizeBytes);
        instance->registry->deserialize(instance->registry->types()[instance->instanceRegistryIndex].typeId(), stream, instance->instance);
    }

}
