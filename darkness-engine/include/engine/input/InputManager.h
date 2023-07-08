#pragma once

#include "engine/primitives/Vector3.h"
#include "containers/memory.h"

#ifndef _DURANGO

namespace gainput
{
    class InputManager;
    class InputMap;
}

namespace engine
{
    enum class InputActions
    {
        Confirm,
        LeftStickX,
        LeftStickY,
        RightStickX,
        RightStickY,
        LeftTrigger,
        RightTrigger,
        LeftBumper,
        RightBumper
    };

    class Camera;

    class InputManager
    {
    public:
        InputManager(int width, int height);
        ~InputManager() {};

        void setCamera(Camera* camera);
        void update();

    private:
        engine::shared_ptr<gainput::InputManager> m_inputManager;

    protected:
        engine::shared_ptr<gainput::InputMap> m_inputMap;
        virtual void updateCamera();
    private:
        
        Camera* m_camera;
        float m_pitch;
        float m_yaw;
        engine::Vector3f m_up;
        engine::Vector3f m_right;
        engine::Vector3f m_forward;
        engine::Vector3f m_upMove;
        engine::Vector3f m_forwardMove;

        
    };
}
#else

namespace DirectX
{
	class GamePad;
}

namespace engine
{
	enum class InputActions
	{
		Confirm,
		LeftStickX,
		LeftStickY,
		RightStickX,
		RightStickY
	};

	class Camera;

	class InputManager
	{
	public:
		InputManager(int width, int height);

		void setCamera(Camera* camera);

		void update();
	private:
		engine::shared_ptr<DirectX::GamePad> m_inputManager;

		Camera* m_camera;
		float m_pitch;
		float m_yaw;
		engine::Vector3f m_up;
		engine::Vector3f m_right;
		engine::Vector3f m_forward;
		engine::Vector3f m_upMove;
		engine::Vector3f m_forwardMove;

		void updateCamera();
	};
}
#endif
