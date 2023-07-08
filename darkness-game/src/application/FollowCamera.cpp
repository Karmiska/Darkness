#include "FollowCamera.h"
#include "components/Transform.h"
#include "engine/Scene.h"

namespace application
{
    FollowCamera::FollowCamera()
        : distance{ this, "Distance", 100.0f }
        , verticalAngle{ this, "Vertical Angle", 70.0f }
        , horizontalAngle{ this, "Horizontal Angle", 90.0f }
        , followSpeed{ this, "Follow Speed", 0.01f }
        , rotationFollowSpeed{ this, "Follow Speed", 0.005f }
        , followRotation{ this, "Follow Rotation", false }
        , centerDeadArea{ this, "Center Dead Area", 14.0f }
        , m_currentHorizontalAngle{ horizontalAngle.value<float>() + 720.0f }
        , m_rotateCamera{ 0 }
        , m_horizontalAngleTarget{ horizontalAngle.value<float>() }
    { 
    }

    void FollowCamera::rotateCamera(int direction)
    {
        m_rotateCamera = direction;
    }

    void FollowCamera::onUpdate(float deltaSeconds)
    {
        if (m_toFollow.expired())
            return;

        auto transform = getComponent<engine::Transform>();
        auto camera = getComponent<engine::Camera>();
        auto cameraPosition = transform->position();
        
        auto toFollowShared = m_toFollow.lock();
        auto toFollowTransform = toFollowShared->getComponent<engine::Transform>();
        auto toFollowPosition = toFollowTransform->position();

        if (m_rotateCamera)
            m_horizontalAngleTarget += static_cast<float>(m_rotateCamera);
            //horizontalAngle.value<float>(horizontalAngle.value<float>() + static_cast<float>(m_rotateCamera));

        auto currentHorizontalAngleValue = horizontalAngle.value<float>();
        horizontalAngle.value<float>(currentHorizontalAngleValue + ((m_horizontalAngleTarget - currentHorizontalAngleValue) * 0.05f));

        if (!followRotation.value<bool>())
        {
            auto rotation = Quaternionf::fromEulerAngles({ 0.0f, horizontalAngle.value<float>(), verticalAngle.value<float>() });
            auto delta = Vector3f{ distance.value<float>(), 0.0f, 0.0f };
            delta = rotation * delta;

            auto newCameraPosition = toFollowPosition + delta;

            auto cameraDelta = newCameraPosition - cameraPosition;
            auto distanceOver = cameraDelta.magnitude() - centerDeadArea.value<float>();
            auto deadPosition = cameraDelta.magnitude() > centerDeadArea.value<float>() ?
                cameraPosition + (cameraDelta.normalize() * distanceOver) :
                cameraPosition;

            camera->position(Vector3f::lerp(cameraPosition, deadPosition, followSpeed.value<float>()));
            camera->rotation(Quaternionf::fromMatrix(Camera::lookAt(newCameraPosition, toFollowPosition, { 0.0f, 1.0f, 0.0f })));
        }
        else
        {
            auto rot = toFollowTransform->rotation();
            auto angles = rot.toEulerAngles();
            
            float targetAngle = horizontalAngle.value<float>() + angles.y + 720.0f;

            float angleDistance = abs(targetAngle - m_currentHorizontalAngle);
            if (abs((targetAngle + 360) - m_currentHorizontalAngle) < angleDistance)
                targetAngle += 360;
            else if (abs((targetAngle - 360) - m_currentHorizontalAngle) < angleDistance)
                targetAngle -= 360;

            m_currentHorizontalAngle = m_currentHorizontalAngle + ((targetAngle - m_currentHorizontalAngle) * rotationFollowSpeed.value<float>());

            //LOG("TargetAngle: %f, CurrentAngle: %f", targetAngle, m_currentHorizontalAngle);

            auto rotation = Quaternionf::fromEulerAngles({ 0.0f, m_currentHorizontalAngle, verticalAngle.value<float>() });
            auto delta = Vector3f{ distance.value<float>(), 0.0f, 0.0f };
            delta = rotation * delta;

            auto newCameraPosition = toFollowPosition + delta;

            auto cameraDelta = newCameraPosition - cameraPosition;
            auto distanceOver = cameraDelta.magnitude() - centerDeadArea.value<float>();
            auto deadPosition = cameraDelta.magnitude() > centerDeadArea.value<float>() ?
                cameraPosition + (cameraDelta.normalize() * distanceOver) :
                cameraPosition;

            camera->position(Vector3f::lerp(cameraPosition, deadPosition, followSpeed.value<float>()));
            camera->rotation(Quaternionf::fromMatrix(Camera::lookAt(newCameraPosition, toFollowPosition, { 0.0f, 1.0f, 0.0f })));
        }
    }

    void FollowCamera::followThisNode(engine::shared_ptr<engine::SceneNode> node)
    {
        m_toFollow = node;
    }

    engine::shared_ptr<engine::EngineComponent> FollowCamera::clone() const
    {
        auto result = engine::make_shared<FollowCamera>();
        result->distance.value<float>(distance.value<float>());
        result->verticalAngle.value<float>(verticalAngle.value<float>());
        result->horizontalAngle.value<float>(horizontalAngle.value<float>());
        result->followSpeed.value<float>(followSpeed.value<float>());
        return result;
    }
}
