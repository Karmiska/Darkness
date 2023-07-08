#include "gtest/gtest.h"
#include "GlobalTestFixture.h"
#include <algorithm>

#ifdef _WIN32
#if 0
#include <Windows.h>
#include <stdio.h>
#include <shellapi.h>
int CALLBACK WinMain(
    _In_ HINSTANCE, // hInstance,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int   //       nCmdShow
)
{
    ::testing::InitGoogleTest(&__argc, __argv);
    
    return RUN_ALL_TESTS();
}
#endif

float step(float a, float b)
{
    return a >= b ? 1.0f : 0.0f;
}

Vector3f step(const Vector3f& a, const Vector3f& b)
{
    return { 
        step(a.x, b.x),
        step(a.y, b.y),
        step(a.z, b.z)
    };
}

Vector3f frac(const Vector3f& vec)
{
    return Vector3f{ 
        vec.x - static_cast<float>(static_cast<int>(vec.x)),
        vec.y - static_cast<float>(static_cast<int>(vec.y)),
        vec.z - static_cast<float>(static_cast<int>(vec.z)) };
}

/*float distanceToNextCell(
    Vector3f position,
    Vector3f direction)
{
    auto zero = Vector3f{ 0.0f, 0.0f, 0.0f };
    auto fr = frac(position);
    auto st = step(zero, direction);
    Vector3f deltas = (st - fr) / direction;
    return std::max(std::min(std::min(deltas.x, deltas.y), deltas.z), std::numeric_limits<float>::epsilon());
}*/

/*float distanceToNextCell(
    Vector3f position,
    Vector3f direction)
{
    auto fr = frac(position);
    auto ds = Vector3f{
        direction.x > 0.0f ? 1.0f - fr.x : fr.x,
        direction.y > 0.0f ? 1.0f - fr.y : fr.y,
        direction.z > 0.0f ? 1.0f - fr.z : fr.z};

    //ds *= direction;
    if ((ds.x < ds.y) && (ds.x < ds.z))
        return position.dot(direction) * ds.x;
    else if ((ds.y < ds.x) && (ds.y < ds.z))
        return position.dot(direction) * ds.y;
    else
        return position.dot(direction) * ds.z;
}*/

/*static const float3 voxelGridSize = float3{ 512.0f, 512.0f, 512.0f };

float3 cell(float3 position, float3 gridSize)
{
    return float3{
        std::floor(position.x / gridSize.x),
        std::floor(position.y / gridSize.y),
        std::floor(position.z / gridSize.z)};
}

float3 cellCount(float level)
{
    return float3{
        voxelGridSize.x / (level == 0.0f ? 1.0f : std::exp2(level)),
        voxelGridSize.y / (level == 0.0f ? 1.0f : std::exp2(level)),
        voxelGridSize.z / (level == 0.0f ? 1.0f : std::exp2(level))};
}

float3 saturate(float3 vec)
{
    return float3{ 
        vec.x < 0.0f ? 0.0f : vec.x > 1.0f ? 1.0f : vec.x,
        vec.y < 0.0f ? 0.0f : vec.y > 1.0f ? 1.0f : vec.y,
        vec.z < 0.0f ? 0.0f : vec.z > 1.0f ? 1.0f : vec.z };
}

float3 intersectionPosition(
    float3 position,
    float3 direction,
    float3 cellId,
    float3 cellCount)
{
    auto crossStep = float3{
        direction.x >= 0.0 ? 1.0f : -1.0f,
        direction.y >= 0.0 ? 1.0f : -1.0f,
        direction.z >= 0.0 ? 1.0f : -1.0f};
    auto crossOffset = crossStep * 0.00001f;
    crossStep = saturate(crossStep);
    
    auto posCell = cell(position, float3{ 512.0f, 512.0f, 512.0f });
    
    // intersect part
    auto cellSize = float3{
        1.0f / cellCount.x,
        1.0f / cellCount.y,
        1.0f / cellCount.z };

    auto planes = cellId / cellCount + cellSize * crossStep;
    auto solutions = (planes - position) / direction;

    auto intersectionPosition = position + direction * std::min(std::min(solutions.x, solutions.y), solutions.z);

    return intersectionPosition;
}*/

float3 intersectPoint(
    float3 point,
    float3 direction, 
    float3 planePoint,
    float3 planeNormal)
{
    float3 diff = point - planePoint;
    float prod1 = diff.dot(planeNormal);
    float prod2 = direction.dot(planeNormal);
    float prod3 = prod1 / (prod2 + std::numeric_limits<float>::epsilon());
    return point - direction * prod3;
}

float3 saturate(float3 vec)
{
    return float3{
        vec.x < 0.0f ? 0.0f : vec.x > 1.0f ? 1.0f : vec.x,
        vec.y < 0.0f ? 0.0f : vec.y > 1.0f ? 1.0f : vec.y,
        vec.z < 0.0f ? 0.0f : vec.z > 1.0f ? 1.0f : vec.z };
}

struct Plane
{
    float3 point;
    float3 normal;
};

float3 intersect(
    float3 position,
    float3 direction)
{
    auto f = frac(position);

    auto plane_normal = float3{ 
        direction.x >= 0.0f ? -1.0f : 1.0f,
        direction.y >= 0.0f ? -1.0f : 1.0f,
        direction.z >= 0.0f ? -1.0f : 1.0f };
    auto plane_point = float3{
        direction.x >= 0.0f ? 1.0f : 0.0f,
        direction.y >= 0.0f ? 1.0f : 0.0f,
        direction.z >= 0.0f ? 1.0f : 0.0f };

    auto ix = intersectPoint(f, direction, float3{ plane_point.x, 0.0f, 0.0f }, float3{ plane_normal.x, 0.0f, 0.0f });
    auto iy = intersectPoint(f, direction, float3{ 0.0f, plane_point.y, 0.0f }, float3{ 0.0f, plane_normal.y, 0.0f });
    auto iz = intersectPoint(f, direction, float3{ 0.0f, 0.0f, plane_point.z }, float3{ 0.0f, 0.0f, plane_normal.z });

    auto xmag = (ix - f).magnitude();
    auto ymag = (iy - f).magnitude();
    auto zmag = (iz - f).magnitude();

    if (xmag < ymag && xmag < zmag)
        return position + (ix - f);
    else if (ymag < xmag && ymag < zmag)
        return position + (iy - f);
    else
        return position + (iz - f);
}

int main(int argc, char* argv[])
{
    float3 pnt{ 1.75f, 1.75f, 1.75f };
    float3 direction{ 1.0f, 0.0f, 0.0f };

    //auto distanceToWall = intersectionPosition(pnt, direction, cell(pnt, voxelGridSize), cellCount(0));
    auto intersectPnt = intersect(pnt, direction);

    ::testing::InitGoogleTest(&argc, argv);
    envPtr = new GlobalEnvironment();
    ::testing::AddGlobalTestEnvironment(envPtr);

    return RUN_ALL_TESTS();
}


#endif
