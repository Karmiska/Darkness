#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"
#include "containers/memory.h"

namespace engine
{
    class ModelPropertiesComponent : public EngineComponent
    {
        Property m_size;

    public:
        ModelPropertiesComponent()
            : m_size{ this, "size", int(1024) }
        {
            m_name = "ModelPropertiesComponent";
        }

        ModelPropertiesComponent(const engine::string& /*path*/)
            : m_size{ this, "size", int(1024) }
        {
            LOG_INFO("TODO: ModelPropertiesComponent does not load actual properties.");
            m_name = "ModelPropertiesComponent";
        }

        engine::shared_ptr<engine::EngineComponent> clone() const override
        {
            return engine::make_shared<ModelPropertiesComponent>();
        }

        void start() override
        {
        }

        int size() const;
        void size(int _size);

    };
}
