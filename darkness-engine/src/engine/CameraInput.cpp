#include "engine/CameraInput.h"
#include <cmath>

namespace engine
{
    CameraInput::CameraInput()
        : m_camera{ nullptr }
        , m_mouseDown{ false }
        , m_lastX{ 0 }
        , m_lastY{ 0 }
        , m_up{ 0.0f, 1.0f, 0.0f }
        , m_right{ 1.0f, 0.0f, 0.0f }
        , m_forward{ 0.0f, 0.0f, 1.0f }
        , m_position{ 0.0f, 0.0f, 0.0f }
        , m_positionCurrent{ 0.0f, 0.0f, 0.0f }
        , m_upMove{ 0.0f, 0.0f, 0.0f }
        , m_forwardMove{ 0.0f, 0.0f, 0.0f }
        , m_upDown{ false }
        , m_downDown{ false }
        , m_rightDown{ false }
        , m_leftDown{ false }
        , m_shiftDown{ false }
        , m_altDown{ false }
        , m_pitch{ 0.0f }
        , m_yaw{ 0.0f }
        , m_pitchCurrent{ 0.0f }
        , m_yawCurrent{ 0.0f }
		, m_speedSteps{ 0 }
		, m_speedMultiplier{ 1.0f }
    {}
    
    CameraInput::CameraInput(engine::shared_ptr<engine::Camera> camera)
        : m_camera{ camera }
        , m_mouseDown{ false }
        , m_lastX{ 0 }
        , m_lastY{ 0 }
        , m_up{ 0.0f, 1.0f, 0.0f }
        , m_right{ 1.0f, 0.0f, 0.0f }
        , m_forward{ 0.0f, 0.0f, 1.0f }
        , m_position{ 0.0f, 0.0f, 0.0f }
        , m_positionCurrent{ 0.0f, 0.0f, 0.0f }
        , m_upMove{ 0.0f, 0.0f, 0.0f }
        , m_forwardMove{ 0.0f, 0.0f, 0.0f }
        , m_upDown{ false }
        , m_downDown{ false }
        , m_rightDown{ false }
        , m_leftDown{ false }
        , m_shiftDown{ false }
        , m_altDown{ false }
        , m_pitch{ 0.0f }
        , m_yaw{ 0.0f }
        , m_pitchCurrent{ 0.0f }
        , m_yawCurrent{ 0.0f }
		, m_speedSteps{ 0 }
		, m_speedMultiplier{ 1.0f }
    {}

    void CameraInput::setCamera(engine::shared_ptr<engine::Camera>& camera)
    {
        m_camera = camera;
    }

    void CameraInput::onMouseMove(int x, int y)
    {
        if (m_camera && m_mouseDown)
        {
            auto d = delta(x, y);

            applyYaw(static_cast<float>(-d.first));
            applyPitch(static_cast<float>(-d.second));
        }

        m_lastX = x;
        m_lastY = y;
    }

    std::pair<int, int> CameraInput::delta(int x, int y) const
    {
        return { x - m_lastX, y - m_lastY };
    }

    void CameraInput::onMouseDown(engine::MouseButton button, int x, int y)
    {
        if (button == engine::MouseButton::Right)
        {
            m_lastX = x;
            m_lastY = y;
            m_mouseDown = true;
        }
    }

    void CameraInput::onMouseUp(engine::MouseButton button, int x, int y)
    {
        if (button == engine::MouseButton::Right)
        {
            m_lastX = x;
            m_lastY = y;
            m_mouseDown = false;
        }
    }

    void CameraInput::onMouseDoubleClick(engine::MouseButton /*button*/, int /*x*/, int /*y*/)
    {

    }

	void CameraInput::onMouseWheel(int /*x*/, int /*y*/, int delta)
	{
		m_speedSteps += delta;
		updateSpeedMultiplier();
	}

	void CameraInput::updateSpeedMultiplier()
	{
		m_speedMultiplier = std::pow(1.2f, static_cast<float>(m_speedSteps));
	}

    void CameraInput::onKeyDown(engine::Key key, const engine::ModifierState& modifierState)
    {
        m_shiftDown = modifierState[engine::KeyModifier::ShiftLeft] || modifierState[engine::KeyModifier::ShiftRight];
        m_altDown = modifierState[engine::KeyModifier::AltLeft] || modifierState[engine::KeyModifier::AltRight];
        switch (key)
        {
        case engine::Key::ArrowUp: { m_upDown = true; break; }
        case engine::Key::ArrowDown: { m_downDown = true; break; }
        case engine::Key::ArrowRight: { m_rightDown = true; break; }
        case engine::Key::ArrowLeft: { m_leftDown = true; break; }

        case engine::Key::W: { m_upDown = true; break; }
        case engine::Key::S: { m_downDown = true; break; }
        case engine::Key::D: { m_rightDown = true; break; }
        case engine::Key::A: { m_leftDown = true; break; }
        default: break;
        }
    }

    void CameraInput::onKeyUp(engine::Key key, const engine::ModifierState& modifierState)
    {
        m_shiftDown = modifierState[engine::KeyModifier::ShiftLeft] || modifierState[engine::KeyModifier::ShiftRight];
        m_altDown = modifierState[engine::KeyModifier::AltLeft] || modifierState[engine::KeyModifier::AltRight];
        switch (key)
        {
        case engine::Key::ArrowUp: { m_upDown = false; break; }
        case engine::Key::ArrowDown: { m_downDown = false; break; }
        case engine::Key::ArrowRight: { m_rightDown = false; break; }
        case engine::Key::ArrowLeft: { m_leftDown = false; break; }

        case engine::Key::W: { m_upDown = false; break; }
        case engine::Key::S: { m_downDown = false; break; }
        case engine::Key::D: { m_rightDown = false; break; }
        case engine::Key::A: { m_leftDown = false; break; }
        default: break;
        }
    }

    bool CameraInput::hasCamera() const
    {
        return (bool)m_camera;
    }

    void CameraInput::position(const engine::Vector3f& position)
    {
        m_position = position;
        m_positionCurrent = m_position;
    }

    void CameraInput::rotation(const engine::Vector3f& rotation)
    {
        m_pitch = rotation.x;
        m_yaw = rotation.y;
        if (m_yaw < 0)
            m_yaw += 360.0f;
        m_pitchCurrent = m_pitch;
        m_yawCurrent = m_yaw;
    }

    engine::Vector3f CameraInput::position() const
    {
        return m_position;
    }

    engine::Vector3f CameraInput::rotation() const
    {
        return engine::Vector3f{ m_pitch, m_yaw, 0.0f };
    }

    void CameraInput::update(float delta)
    {
        float move = delta * MoveSpeed * m_speedMultiplier;
        if (m_shiftDown)
            move = delta * FastMoveSpeed * m_speedMultiplier;

        if (m_upDown)
            m_position -= m_forwardMove * move;
        if (m_downDown)
            m_position += m_forwardMove * move;

        if (m_rightDown)
            m_position += m_right * move;
        if (m_leftDown)
            m_position -= m_right * move;

        if (m_camera)
            apply();
    }

    std::pair<int, int> CameraInput::mousePosition() const
    {
        return { m_lastX, m_lastY };
    }

    void CameraInput::closeInOnYaw(const float& target, float& current, const float& range)
    {
        constexpr float steps = 3.3f;
        float halfRange = range / 2.0f;

        float right = target + halfRange;
        bool onTheRight = false;
        float distance = 0.0f;
        if (right > range)
        {
            right -= range;
            if (current >= 0 && current < right)
            {
                // current is on the right of target
                // looped around 0
                distance = (range - target) + current;
                onTheRight = true;
            }
            else if (current >= target && current < range)
            {
                // current is on the right of target
                distance = current - target;
                onTheRight = true;
            }
            else
            {
                // current is on left from target
                if (current <= target)
                {
                    distance = target - current;
                    onTheRight = false;
                }
                else
                {
                    distance = target + (range - current);
                    onTheRight = false;
                }
            }
        }
        else
        {
            // current is on the right of target
            if (current >= target && current < right)
            {
                distance = current - target;
                onTheRight = true;
            }
            else
            {
                // current on left
                if (current < target)
                {
                    distance = target - current;
                    onTheRight = false;
                }
                else
                {
                    distance = target + (range - current);
                    onTheRight = false;
                }
            }
        }

        if (distance > range)
        {
            LOG("distance: %f", distance);
        }

        float dd = distance / steps;
        if (onTheRight)
        {
            current -= dd;
            if (current < 0)
                current += range;
        }
        else
        {
            current += dd;
            if (current >= range)
                current -= range;
        }

        if (current < 0)
        {
            LOG("the hell");
        }
    }

    void CameraInput::closeInOnPitch(const float& target, float& current)
    {
        float distance = 0.0f;
        constexpr float steps = 4.0f;
        if (target > current)
        {
            distance = target - current;
            float dd = distance / steps;
            current += dd;
        }
        else
        {
            distance = current - target;
            float dd = distance / steps;
            current -= dd;
        }
    }

    void CameraInput::applyYaw(float angle)
    {
        m_yaw += angle * 0.3f;
        if (m_yaw < 0.0f)
            m_yaw += 360.0f;
        if (m_yaw >= 360.0f)
            m_yaw -= 360.0f;
    };

    void CameraInput::applyPitch(float angle)
    {
        m_pitch += angle * 0.3f;
        if (m_pitch < -90.0f)
            m_pitch = -90.0f;
        else if (m_pitch > 90.0f)
            m_pitch = 90.0f;
    };

    void CameraInput::apply(bool updateCamera)
    {
        closeInOnPitch(m_pitch, m_pitchCurrent);
        closeInOnYaw(m_yaw, m_yawCurrent, 360.0f);

        float sp = std::sinf(DEG_TO_RAD * m_pitchCurrent);
        float cp = std::cosf(DEG_TO_RAD * m_pitchCurrent);
        float sy = std::sinf(DEG_TO_RAD * m_yawCurrent);
        float cy = std::cosf(DEG_TO_RAD * m_yawCurrent);

        m_right = { cy, 0.0f, -sy };
        m_forward = { sy * cp, -sp, cy * cp };
        m_up = m_forward.cross(m_right);

        m_upMove = { 0.0f, 1.0f, 0.0f };

        //m_forwardMove = { sy, 0.0f, cy };
        m_forwardMove = m_forward;

        engine::Vector3f distance = m_position - m_positionCurrent;
        m_positionCurrent += distance / 3.3f;

        engine::Matrix4f modelMatrix{
            m_right.x,        m_right.y,        m_right.z,        0.0f,
            m_up.x,            m_up.y,            m_up.z,            0.0f,
            m_forward.x,    m_forward.y,    m_forward.z,    0.0f,
            m_positionCurrent.x,    m_positionCurrent.y,    m_positionCurrent.z,    1.0f };

        engine::Matrix4f viewMatrix{
            m_right.x,        m_up.x,            m_forward.x,    0.0f,
            m_right.y,        m_up.y,            m_forward.y,    0.0f,
            m_right.z,        m_up.z,            m_forward.z,    0.0f,
            m_positionCurrent.dot(m_right),    -m_positionCurrent.dot(m_up),    m_positionCurrent.dot(m_forward),    1.0f };

        if (updateCamera)
        {
            m_camera->rotation(engine::Quaternionf::fromMatrix(viewMatrix));
            m_camera->position(m_positionCurrent);
        }

        m_camera->forward(m_forward);
        m_camera->up(m_up);
        m_camera->right(m_right);
    }

    void CameraInput::moveAlongSideAxis(float t)
    {
        m_position += m_right * t;
    }
    // --------------------------------------------------------------------------------------
    void CameraInput::moveAlongUpAxis(float t)
    {
        m_position += m_upMove * t;
    }
    // --------------------------------------------------------------------------------------
    void CameraInput::moveAlongForwardAxis(float t)
    {
        m_position += m_forwardMove * t;
    }

    void CameraInput::orthonormalize()
    {
        m_forward.normalize();
        m_right = m_up.cross(m_forward);
        m_right.normalize();
        m_up = m_forward.cross(m_right);
    }
}
