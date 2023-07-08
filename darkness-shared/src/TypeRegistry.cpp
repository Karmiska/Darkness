#include "TypeRegistry.h"
#include <algorithm>

namespace serialization
{
    Type& TypeRegistry::getType(const engine::string& typeId)
    {
        auto index = std::find_if(
            m_types.begin(), 
            m_types.end(), 
            [typeId](Type type) 
        { return type.typeId() == typeId; });
        return *index;
    }

    void TypeRegistry::registerType(Type type)
    {
        m_types.emplace_back(type);
    };

    void* TypeRegistry::newType(const engine::string& type)
    {
        return getType(type).construct();
    }

    void TypeRegistry::deleteType(const engine::string& type, void* ptr)
    {
        getType(type).destroy(ptr);
    }

    void TypeRegistry::serialize(const engine::string& type, Stream& stream, void* ptr)
    {
        return getType(type).serialize(stream, ptr);
    }

    void TypeRegistry::deserialize(const engine::string& type, Stream& stream, void* ptr)
    {
        getType(type).deserialize(stream, ptr);
    }

    void TypeRegistry::registerProperty(const engine::string& type, engine::shared_ptr<Property> typeProperty)
    {
        getType(type).registerProperty(typeProperty);
    }
}
