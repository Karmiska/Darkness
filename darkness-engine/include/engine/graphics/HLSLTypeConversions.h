#pragma once

#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"

#include "engine/primitives/Matrix3.h"
#include "engine/primitives/Matrix4.h"

#include "shaders/ShaderPodTypes.h"

using float2 = engine::Vector2f;
using float3 = engine::Vector3f;
using float4 = engine::Vector4f;

using float3x3 = engine::Matrix3f;
using float4x4 = engine::Matrix4f;

using uint = uint32_t;
using uint2 = engine::Uint2;
using uint3 = engine::Uint3;
using uint4 = engine::Uint4;

namespace engine
{
    using float2 = Vector2f;
    using float3 = Vector3f;
    using float4 = Vector4f;

    using float3x3 = Matrix3f;
    using float4x4 = Matrix4f;

    using uint = uint32_t;
}
