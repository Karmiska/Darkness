#include "CharacterInputManager.h"
#include "engine/Scene.h"
#include "components/Transform.h"

#ifndef _DURANGO
#include <gainput/gainput.h>

namespace application
{
    using namespace gainput;

    CharacterInputManager::CharacterInputManager(int width, int height)
        : InputManager{ width, height }
    {}

    void CharacterInputManager::setCharacterNode(engine::shared_ptr<engine::SceneNode> character)
    {
        m_characterNode = character;
    }

    void CharacterInputManager::setCameraNode(engine::shared_ptr<engine::SceneNode> camera)
    {
        m_cameraNode = camera;
    }

    void CharacterInputManager::setCameraRotateCallback(std::function<void(int)> onCameraRotateInput)
    {
        m_onCameraRotateInput = onCameraRotateInput;
    }

    void CharacterInputManager::updateCamera()
    {
        if (m_characterNode.expired())
            return;

        //auto rightStickX = m_inputMap->GetFloat(static_cast<UserButtonId>(engine::InputActions::RightStickX));
        //auto rightStickY = m_inputMap->GetFloat(static_cast<UserButtonId>(engine::InputActions::RightStickY));

        auto leftBumper = m_inputMap->GetBool(static_cast<UserButtonId>(engine::InputActions::LeftBumper));
        auto rightBumper = m_inputMap->GetBool(static_cast<UserButtonId>(engine::InputActions::RightBumper));
        int bumperDirection = 0;
        if (leftBumper) bumperDirection += 1;
        if (rightBumper) bumperDirection -= 1;
        if (m_onCameraRotateInput)
            m_onCameraRotateInput(bumperDirection);


        auto leftStickX = m_inputMap->GetFloat(static_cast<UserButtonId>(engine::InputActions::LeftStickX));
        auto leftStickY = m_inputMap->GetFloat(static_cast<UserButtonId>(engine::InputActions::LeftStickY));
        auto directionVector = Vector3f{ leftStickX, 0.0f, leftStickY };

        auto cameraNode = m_cameraNode.lock();
        if (cameraNode)
        {
            // screen relative movement
            auto camera = cameraNode->getComponent<Camera>();
            auto cameraForward = camera->forward();
            auto normalizedCameraForward = Vector2f(cameraForward.x, cameraForward.z).normalize();
            auto cameraDirection = Vector3f(normalizedCameraForward.x, 0.0f, normalizedCameraForward.y);
            auto cameraAngle = (atan2(cameraDirection.x, cameraDirection.z) / static_cast<float>(TWO_PI)) * 360.0f;
            auto directionAngle = (atan2(directionVector.x, -directionVector.z) / static_cast<float>(TWO_PI)) * 360.0f;

            if (cameraAngle < 0) cameraAngle += 360.0f;
            if (directionAngle < 0) directionAngle += 360.0f;


            //LOG("cameraAngle degrees: %f, directionAngle degrees: %f", cameraAngle, directionAngle);

            auto targetAngle = directionAngle + cameraAngle - 90.0f;
            auto targetRadians = (targetAngle / 360.0f) * static_cast<float>(TWO_PI);
            directionVector = Vector3f{ std::cos(targetRadians), 0.0f, std::sin(targetRadians) } * directionVector.magnitude() * -1.0f;
        }

        auto canRotate = directionVector.magnitude() > 0.04;
        directionVector *= 0.15;
        
        auto characterNode = m_characterNode.lock();
        auto characterTransform = characterNode->getComponent<Transform>();
        auto characterPosition = characterTransform->position();
        auto characterRotation = characterTransform->rotation();
        
        auto characterTarget = characterPosition + Vector3f{ directionVector.x, directionVector.y, -directionVector.z };
        auto newCharacterRotation = Camera::lookAt(characterPosition, characterTarget);
        if(canRotate)
            characterTransform->rotation(Quaternionf::fromMatrix(newCharacterRotation));
        characterTransform->position(characterPosition + Vector3f{ -directionVector.x, directionVector.y, directionVector.z });
    }
}
#endif
