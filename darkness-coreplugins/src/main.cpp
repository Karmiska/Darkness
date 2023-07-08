#include "Initialization.h"
#include "TypeRegistry.h"
#include "Serialization.h"
#include "Transform.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

using namespace serialization;

DLLEXPORT void CALLCONV initializeTypes(void* typeRegistry)
{
    TypeRegistry* registry = reinterpret_cast<TypeRegistry*>(typeRegistry);

    registry->registerType<engine::Transform>(
        []()->void* { return new engine::Transform(); },
        [](void* ptr) { delete reinterpret_cast<engine::Transform*>(ptr); },
        [](Stream& stream, void* ptr) { serialize(stream, *reinterpret_cast<engine::Transform*>(ptr)); },
        [](Stream& stream, void* ptr) { deserialize(stream, *reinterpret_cast<engine::Transform*>(ptr)); }
    );

    registry->registerProperty<engine::Transform>(engine::make_shared<Property>("position", engine::Vector3f{ 0.0f, 0.0f, 0.0f }));
    registry->registerProperty<engine::Transform>(engine::make_shared<Property>("rotation", engine::Matrix4f::identity() ));
    registry->registerProperty<engine::Transform>(engine::make_shared<Property>("scale", engine::Vector3f{ 1.0f, 1.0f, 1.0f }));
    registry->registerProperty<engine::Transform>(engine::make_shared<Property>("name", engine::string("Transform")));
}

#include "main.h"

BOOL WINAPI DllMain(
    _In_ HINSTANCE /*hinstDLL*/,
    _In_ DWORD     /*fdwReason*/,
    _In_ LPVOID    /*lpvReserved*/
)
{
    forceFunctionInclude();
    return true;
}
