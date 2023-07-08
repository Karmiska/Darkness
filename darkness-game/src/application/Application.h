#pragma once

#include "platform/window/Window.h"
#include "engine/Engine.h"
#include "platform/network/SocketServer.h"
#include "containers/memory.h"

namespace application
{
    static int ScreenWidth = 1920;
    static int ScreenHeight = 1080;
    class CharacterInputManager;
    class FollowCamera;

    class Application
    {
    public:
        Application();

    private:
        engine::shared_ptr<platform::Window> m_window;
        engine::shared_ptr<CharacterInputManager> m_characterInputManager;
        Engine m_engine;
        void attachFollowCamera(
            engine::shared_ptr<SceneNode> camera,
            engine::shared_ptr<SceneNode> character);
        void attachInputManager(
            engine::shared_ptr<SceneNode> camera,
            engine::shared_ptr<SceneNode> character);

        engine::shared_ptr<FollowCamera> m_followCameraComponent;
    };
}
