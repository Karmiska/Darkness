#pragma once

#include "containers/unordered_map.h"
#include "TypeFactoryIf.h"

namespace engine
{
    class TypeFactory : public TypeFactoryIf
    {
    public:
        void registerType(
            const engine::string& typeName,
            CreateFunction create,
            DestroyFunction destroy) override;

        void unregisterType(
            const engine::string& typeName
        ) override;

        void* createType(const engine::string& typeName) const;
        void destroyType(const engine::string& typeName, void* typeInstance) const;

        const engine::unordered_map<engine::string, TypeContainer>& types() const override;
    private:
        
        engine::unordered_map<engine::string, TypeContainer> m_types;
    };
}
