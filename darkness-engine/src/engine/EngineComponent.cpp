#include "engine/EngineComponent.h"
#include "tools/Property.h"
#include "engine/Scene.h"
#include "tools/Serialization.h"

namespace engine
{
    engine::string EngineComponent::name() const
    {
        return m_name;
    }

    void EngineComponent::name(const engine::string& name) 
    {
        m_name = name;
    }

    void EngineComponent::parentNode(SceneNode* node)
    {
        m_parentNode = node;
    }

    SceneNode* EngineComponent::parentNode() const
    {
        return m_parentNode;
    }

    engine::vector<engine::shared_ptr<EngineComponent>> EngineComponent::getComponents()
    {
        engine::vector<engine::shared_ptr<EngineComponent>> result;
        for (size_t i = 0; i < m_parentNode->componentCount(); ++i)
        {
            result.emplace_back(m_parentNode->component(i));
        }
        return result;
    }

    void EngineComponent::writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
    {
        writer.StartObject();

        engine::string key = serialization::keyPrefix<serialization::KeyTypes, serialization::TypeId>(serialization::ComponentName);
        writer.Key(key.data());
        writer.String(m_name.data());

        key = serialization::keyPrefix<serialization::KeyTypes, serialization::TypeId>(serialization::PropertyList);
        writer.Key(key.data());

        writer.StartArray();
        for (const auto& prop : m_properties)
        {
            writer.StartObject();
            prop.second->writeJson(writer);
            writer.EndObject();
        }

        writer.EndArray();
        writer.EndObject();
    }
}
