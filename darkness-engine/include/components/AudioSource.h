#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"

namespace engine
{
    class Transform;

    class AudioSourceComponent : public EngineComponent
    {
        Property Source;
    public:
        AudioSourceComponent();
        engine::shared_ptr<engine::EngineComponent> clone() const override;
    private:
        engine::shared_ptr<Transform> m_transform;

    };
}
