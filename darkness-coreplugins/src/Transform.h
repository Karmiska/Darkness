#pragma once

#include "containers/string.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"
#include "Property.h"

namespace engine
{
    class Transform
    {
        Property m_position;
        Property m_rotation;
        Property m_scale;
        Property m_name;
    public:
        Transform()
            : m_position{ "position", Vector3f{ 0.0f, 0.0f, 0.0f } }
            , m_rotation{ "rotation", Matrix4f::identity() }
            , m_scale{ "scale", Vector3f{ 1.0f, 1.0f, 1.0f } }
            , m_name{ "name", engine::string("Transform") }
        {
        }

        Vector3f& position() { return m_position.value<Vector3f>(); }
        const Vector3f& position() const { return m_position.value<Vector3f>(); }
        void position(const Vector3f& vec) { m_position.value<Vector3f>() = vec; }

        Matrix4f& rotation() { return m_rotation.value<Matrix4f>(); }
        const Matrix4f& rotation() const { return m_rotation.value<Matrix4f>(); }
        void rotation(const Matrix4f& mat) { m_rotation.value<Matrix4f>() = mat; }

        Vector3f& scale() { return m_scale.value<Vector3f>(); }
        const Vector3f& scale() const { return m_scale.value<Vector3f>(); }
        void scale(const Vector3f& vec) { m_scale.value<Vector3f>() = vec; }
    };
}
