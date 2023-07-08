#pragma once

#include "containers/unordered_map.h"
#include "containers/string.h"
#include <functional>

namespace engine
{
    using CreateFunction = std::function<void*()>;
    using DestroyFunction = std::function<void(void*)>;

    struct TypeContainer
    {
        engine::string typeName;
        CreateFunction create;
        DestroyFunction destroy;
    };

    class TypeFactoryIf
    {
    public:
        virtual void registerType(
            const engine::string& typeName,
            CreateFunction create,
            DestroyFunction destroy) = 0;

        virtual void unregisterType(const engine::string& typeName) = 0;

        virtual const engine::unordered_map<engine::string, TypeContainer>& types() const = 0;
    };
}
