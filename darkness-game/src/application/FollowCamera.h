#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"
#include "containers/memory.h"

namespace engine
{
    class SceneNode;
}

namespace application
{
    class FollowCamera : public engine::EngineComponent
    {
        engine::Property distance;
        engine::Property verticalAngle;
        engine::Property horizontalAngle;
        engine::Property followSpeed;
        engine::Property rotationFollowSpeed;
        engine::Property followRotation;
        engine::Property centerDeadArea;
    public:
        FollowCamera();

        void followThisNode(engine::shared_ptr<engine::SceneNode> node);
        void rotateCamera(int direction);

        engine::shared_ptr<engine::EngineComponent> clone() const override;

    protected:
        void onUpdate(float deltaSeconds) override;

    private:
        std::weak_ptr<engine::SceneNode> m_toFollow;
        float m_currentHorizontalAngle;
        int m_rotateCamera;
        float m_horizontalAngleTarget;
    };
}