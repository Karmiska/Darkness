#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"

namespace engine
{
    class Transform : public EngineComponent
    {
        Property m_position;
        Property m_rotation;
        Property m_scale;

        bool m_positionChanged;
        bool m_rotationChanged;
        bool m_scaleChanged;
    public:
        Transform()
            : m_position{ this, "position", Vector3f{ 0.0f, 0.0f, 0.0f }, [this]() { this->m_positionChanged = true; } }
            , m_rotation{ this, "rotation", Quaternionf::fromEulerAngles(0.0f, 0.0f, 0.0f), [this]() { this->m_rotationChanged = true; } }
            , m_scale { this, "scale", Vector3f{ 1.0f, 1.0f, 1.0f }, [this]() { this->m_scaleChanged = true; } }
            , m_positionChanged{ true }
            , m_rotationChanged{ true }
            , m_scaleChanged{ true }
        {
            m_name = "Transform";
        }

        const Vector3f& position() const { return m_position.value<Vector3f>(); }
        void position(const Vector3f& vec)
        {
            auto dif = vec != m_position.value<Vector3f>();
            if(dif)
                m_position.value<Vector3f>(vec);
        }
        bool positionChanged(bool clear = false)
        {
            bool res = m_positionChanged;
            if (clear) m_positionChanged = false;
            return res;
        }

        const Quaternionf& rotation() const { return m_rotation.value<Quaternionf>(); }
        void rotation(const Quaternionf& mat) { m_rotation.value<Quaternionf>(mat); }
        bool rotationChanged(bool clear = false)
        {
            bool res = m_rotationChanged;
            if (clear) m_rotationChanged = false;
            return res;
        }

        const Vector3f& scale() const { return m_scale.value<Vector3f>(); }
        void scale(const Vector3f& vec) { m_scale.value<Vector3f>(vec); }
        bool scaleChanged(bool clear = false)
        {
            bool res = m_scaleChanged;
            if (clear) m_scaleChanged = false;
            return res;
        }

        Matrix4f transformMatrix() const
        {
            return Matrix4f::translate(m_position.value<Vector3f>()) * m_rotation.value<Quaternionf>().toMatrix() * Matrix4f::scale(m_scale.value<Vector3f>());
        }

        engine::shared_ptr<engine::EngineComponent> clone() const override
        {
            auto transform = engine::make_shared<engine::Transform>();
            transform->position(position());
            transform->rotation(rotation());
            transform->scale(scale());
            return transform;
        }
    };
}
