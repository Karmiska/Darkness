#include "components/CollisionShapeComponent.h"
#include "btBulletDynamicsCommon.h"

namespace engine
{
    engine::string collisionShapeToString(const CollisionShape& shape)
    {
        switch (shape)
        {
        case CollisionShape::Plane: return "Plane";
        case CollisionShape::Box: return "Box";
        case CollisionShape::Capsule: return "Capsule";
        case CollisionShape::Sphere: return "Sphere";
        case CollisionShape::Cylinder: return "Cylinder";
        case CollisionShape::Cone: return "Cone";
        case CollisionShape::BvhTriangleMesh: return "BvhTriangleMesh";
        case CollisionShape::ConvexHull: return "ConvexHull";
        case CollisionShape::ConvexTriangleMesh: return "ConvexTriangleMesh";
        }
        return "Plane";
    }

    CollisionShape stringToCollisionShape(const engine::string& shape)
    {
        if (shape == "Plane")
            return CollisionShape::Plane;
        else if (shape == "Box")
            return CollisionShape::Box;
        else if (shape == "Capsule")
            return CollisionShape::Capsule;
        else if (shape == "Sphere")
            return CollisionShape::Sphere;
        else if (shape == "Cylinder")
            return CollisionShape::Cylinder;
        else if (shape == "Cone")
            return CollisionShape::Cone;
        else if (shape == "BvhTriangleMesh")
            return CollisionShape::BvhTriangleMesh;
        else if (shape == "ConvexHull")
            return CollisionShape::ConvexHull;
        else if (shape == "ConvexTriangleMesh")
            return CollisionShape::ConvexTriangleMesh;
        return CollisionShape::Plane;
    }


    CollisionShapeComponent::CollisionShapeComponent()
        : m_shape{ this, "Shape", CollisionShape::Box, [this]() { this->refreshShape(); } }
    {
        m_name = "CollisionShape";
        refreshShape();
    }

    void CollisionShapeComponent::refreshShape()
    {
        //m_collisionShape = engine::make_shared<btBoxShape>(btVector3(btScalar(1.), btScalar(1.), btScalar(1.)));
#if 1
        switch (m_shape.value<CollisionShape>())
        {
        case CollisionShape::Plane: { m_collisionShape = engine::make_shared<btStaticPlaneShape>(btVector3{ 0.0f, 1.0f, 0.0f }, 0.0f); break; }
        case CollisionShape::Box: { m_collisionShape = engine::make_shared<btBoxShape>(btVector3(btScalar(1.), btScalar(1.), btScalar(1.))); break; }
        case CollisionShape::Capsule: { m_collisionShape = engine::make_shared<btCapsuleShape>(1.0f, 1.0f); break; }
        case CollisionShape::Sphere: { m_collisionShape = engine::make_shared<btSphereShape>(1.0f); break; }
        case CollisionShape::Cylinder: { m_collisionShape = engine::make_shared<btCylinderShape>(btVector3(0.5f, 0.5f, 0.5f)); break; }
        case CollisionShape::Cone: { m_collisionShape = engine::make_shared<btConeShape>(1.0f, 1.0f); break; }

        case CollisionShape::BvhTriangleMesh: { /*m_collisionShape = engine::make_shared<btBvhTriangleMeshShape>();*/ break; }
        case CollisionShape::ConvexHull: { /*m_collisionShape = engine::make_shared<btConvexHullShape>();*/ break; }
        case CollisionShape::ConvexTriangleMesh: { /*m_collisionShape = engine::make_shared<btConvexTriangleMeshShape>();*/ break; }
        }
#endif
    }

    engine::shared_ptr<engine::EngineComponent> CollisionShapeComponent::clone() const
    {
        auto res = engine::make_shared<engine::CollisionShapeComponent>();
        res->m_shape.value<CollisionShape>(m_shape.value<CollisionShape>());
        return res;
    }
}
