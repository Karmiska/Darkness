#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"

class btRigidBody;

namespace engine
{
    class Transform;
    class CollisionShapeComponent;

    class RigidBodyComponent : public EngineComponent
    {
        Property m_mass;
    public:
        RigidBodyComponent();
        engine::shared_ptr<engine::EngineComponent> clone() const override;

        engine::shared_ptr<btRigidBody> body();
    private:
        engine::shared_ptr<Transform> m_transform;
        engine::shared_ptr<CollisionShapeComponent> m_collisionShape;

        engine::shared_ptr<btRigidBody> m_rigidBody;
        
    };
}
