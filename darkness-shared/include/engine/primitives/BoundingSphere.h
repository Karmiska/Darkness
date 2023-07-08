#pragma once

#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"

namespace engine
{
    struct BoundingSphere
    {
        Vector4f positionRadius;
        
        BoundingSphere()
            : positionRadius{ 0.0f, 0.0f, 0.0f, 0.0f }
        {};

        BoundingSphere(Vector4f positionRadius)
            : positionRadius{ positionRadius }
        {};

        BoundingSphere(const Vector3f& position, float radius)
            : positionRadius{ position.x, position.y, position.z, radius }
        {};

        float radius() const { return positionRadius.w; };
        Vector3f position() const { return positionRadius.xyz(); };
    };
}
