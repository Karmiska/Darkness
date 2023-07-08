#pragma once

#include "containers/memory.h"
#include "components/Camera.h"
#include "engine/primitives/Matrix4.h"
#include "engine/InputEvents.h"

constexpr float MoveSpeed = 2.0f;
constexpr float FastMoveSpeed = 10.0f;

namespace engine
{
    class CameraInput
    {
    public:
        CameraInput();
        CameraInput(engine::shared_ptr<engine::Camera> camera);

        void setCamera(engine::shared_ptr<engine::Camera>& camera);

        void onMouseMove(int x, int y);
        void onMouseDown(engine::MouseButton button, int x, int y);
        void onMouseUp(engine::MouseButton button, int x, int y);
        void onMouseDoubleClick(engine::MouseButton button, int x, int y);
		void onMouseWheel(int x, int y, int delta);

        void onKeyDown(engine::Key key, const engine::ModifierState& modifierState);
        void onKeyUp(engine::Key key, const engine::ModifierState& modifierState);

        bool hasCamera() const;

        void position(const engine::Vector3f& position);
        void rotation(const engine::Vector3f& rotation);

        engine::Vector3f position() const;
        engine::Vector3f rotation() const;

        void update(float delta);

        std::pair<int, int> mousePosition() const;
    private:
        engine::shared_ptr<engine::Camera> m_camera;
        bool m_mouseDown;
        int m_lastX;
        int m_lastY;
        std::pair<int, int> delta(int x, int y) const;

        engine::Vector3f m_up;
        engine::Vector3f m_right;
        engine::Vector3f m_forward;

        engine::Vector3f m_position;
        engine::Vector3f m_upMove;
        engine::Vector3f m_forwardMove;

        engine::Vector3f m_positionCurrent;

        bool m_upDown;
        bool m_downDown;
        bool m_rightDown;
        bool m_leftDown;
        bool m_shiftDown;
        bool m_altDown;

        float m_pitch;
        float m_yaw;

        float m_pitchCurrent;
        float m_yawCurrent;

		int m_speedSteps;
		float m_speedMultiplier;

        void closeInOnYaw(const float& target, float& current, const float& range);
        void closeInOnPitch(const float& target, float& current);
        void applyYaw(float angle);
        void applyPitch(float angle);

    public:
        void apply(bool updateCamera = true);
    private:
        void moveAlongSideAxis(float t);
        void moveAlongUpAxis(float t);
        void moveAlongForwardAxis(float t);
        void orthonormalize();
		void updateSpeedMultiplier();
    };
}
