#pragma once

#include "containers/memory.h"
#include "engine/input/InputManager.h"

namespace engine
{
    class SceneNode;
}

namespace application
{
    class CharacterInputManager : public engine::InputManager
    {
    public:
        CharacterInputManager(int width, int height);
        void setCharacterNode(engine::shared_ptr<engine::SceneNode> character);
        void setCameraNode(engine::shared_ptr<engine::SceneNode> camera);
        void setCameraRotateCallback(std::function<void(int)> onCameraRotateInput);
    protected:
        void updateCamera() override;

    private:
        std::weak_ptr<engine::SceneNode> m_characterNode;
        std::weak_ptr<engine::SceneNode> m_cameraNode;
        std::function<void(int)> m_onCameraRotateInput;
    };
}
