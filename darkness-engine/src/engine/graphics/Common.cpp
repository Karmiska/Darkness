#include "engine/graphics/Common.h"
#include "engine/graphics/CommandList.h"
#include "components/Camera.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"
#include "engine/primitives/Matrix4.h"
#include <algorithm>

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12GpuMarker.h"
#include "engine/graphics/dx12/DX12CpuMarker.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanGpuMarker.h"
#include "engine/graphics/vulkan/VulkanCpuMarker.h"
#endif

std::atomic<uint64_t> GlobalUniqueHandleId = 1;

namespace engine
{

#if 0
    void NormalizePlane(Vector4f& plane)
    {
        float mag;
        mag = sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        plane.x = plane.x / mag;
        plane.y = plane.y / mag;
        plane.z = plane.z / mag;
        plane.w = plane.w / mag;
    }

    engine::vector<Vector4f> extractFrustumPlanes(const Matrix4f& modelViewProjection, bool normalize)
    {
        engine::vector<Vector4f> result(6);
        // Left clipping plane
        result[0].x = modelViewProjection.m22 + modelViewProjection.m00;
        result[0].y = modelViewProjection.m13 + modelViewProjection.m10;
        result[0].z = modelViewProjection.m23 + modelViewProjection.m20;
        result[0].w = modelViewProjection.m33 + modelViewProjection.m30;
        // Right clipping plane
        result[1].x = modelViewProjection.m22 - modelViewProjection.m00;
        result[1].y = modelViewProjection.m13 - modelViewProjection.m10;
        result[1].z = modelViewProjection.m23 - modelViewProjection.m20;
        result[1].w = modelViewProjection.m33 - modelViewProjection.m30;
        // Top clipping plane
        result[2].x = modelViewProjection.m22 - modelViewProjection.m01;
        result[2].y = modelViewProjection.m13 - modelViewProjection.m11;
        result[2].z = modelViewProjection.m23 - modelViewProjection.m21;
        result[2].w = modelViewProjection.m33 - modelViewProjection.m31;
        // Bottom clipping plane
        result[3].x = modelViewProjection.m22 + modelViewProjection.m01;
        result[3].y = modelViewProjection.m13 + modelViewProjection.m11;
        result[3].z = modelViewProjection.m23 + modelViewProjection.m21;
        result[3].w = modelViewProjection.m33 + modelViewProjection.m31;
        // Near clipping plane
        result[4].x = modelViewProjection.m02;
        result[4].y = modelViewProjection.m12;
        result[4].z = modelViewProjection.m22;
        result[4].w = modelViewProjection.m32;
        // Far clipping plane
        result[5].x = modelViewProjection.m22 - modelViewProjection.m02;
        result[5].y = modelViewProjection.m13 - modelViewProjection.m12;
        result[5].z = modelViewProjection.m23 - modelViewProjection.m22;
        result[5].w = modelViewProjection.m33 - modelViewProjection.m32;
        // Normalize the plane equations, if requested
        if (normalize == true)
        {
            NormalizePlane(result[0]);
            NormalizePlane(result[1]);
            NormalizePlane(result[2]);
            NormalizePlane(result[3]);
            NormalizePlane(result[4]);
            NormalizePlane(result[5]);
        }
        return result;
    }
#endif

    ResourceState getResourceStateFromUsage(ResourceUsage usage)
    {
        ResourceState resState;
        if (usage == ResourceUsage::Upload)
        {
            resState = ResourceState::GenericRead;
        }
        else if (usage == ResourceUsage::GpuToCpu)
        {
            resState = ResourceState::CopyDest;
        }
        else if (usage == ResourceUsage::GpuRead)
        {
            resState = ResourceState::CopyDest;
        }
        else if (usage == ResourceUsage::DepthStencil)
        {
            resState = ResourceState::DepthWrite;
        }
        else if (usage == ResourceUsage::GpuRenderTargetReadWrite)
        {
            resState = ResourceState::RenderTarget;
        }
        else if (usage == ResourceUsage::GpuRenderTargetRead)
        {
            resState = ResourceState::RenderTarget;
        }
        else if (usage == ResourceUsage::AccelerationStructure)
        {
            resState = ResourceState::Common;
        }
        else
        {
            resState = ResourceState::Common;
        }
        return resState;
    };

#if 1
    engine::vector<Vector4f> extractFrustumPlanes(const Matrix4f& modelViewProjection, bool normalize)
    {
        engine::vector<Vector4f> result(6);

        // column major matrix
        // Left Plane
        // col4 + col1
        /*result[0].x = modelViewProjection[3] + modelViewProjection[0];
        result[0].y = modelViewProjection[7] + modelViewProjection[4];
        result[0].z = modelViewProjection[11] + modelViewProjection[8];
        result[0].w = modelViewProjection[15] + modelViewProjection[12];

        // Right Plane
        // col4 - col1
        result[1].x = modelViewProjection[3] - modelViewProjection[0];
        result[1].y = modelViewProjection[7] - modelViewProjection[4];
        result[1].z = modelViewProjection[11] - modelViewProjection[8];
        result[1].w = modelViewProjection[15] - modelViewProjection[12];

        // Bottom Plane
        // col4 + col2
        result[2].x = modelViewProjection[3] + modelViewProjection[1];
        result[2].y = modelViewProjection[7] + modelViewProjection[5];
        result[2].z = modelViewProjection[11] + modelViewProjection[9];
        result[2].w = modelViewProjection[15] + modelViewProjection[13];

        // Top Plane
        // col4 - col2
        result[3].x = modelViewProjection[3] - modelViewProjection[1];
        result[3].y = modelViewProjection[7] - modelViewProjection[5];
        result[3].z = modelViewProjection[11] - modelViewProjection[9];
        result[3].w = modelViewProjection[15] - modelViewProjection[13];

        // Near Plane
        // col4 + col3
        result[4].x = modelViewProjection[3] + modelViewProjection[2];
        result[4].y = modelViewProjection[7] + modelViewProjection[6];
        result[4].z = modelViewProjection[11] + modelViewProjection[10];
        result[4].w = modelViewProjection[15] + modelViewProjection[14];

        // Far Plane
        // col4 - col3
        result[5].x = modelViewProjection[3] - modelViewProjection[2];
        result[5].y = modelViewProjection[7] - modelViewProjection[6];
        result[5].z = modelViewProjection[11] - modelViewProjection[10];
        result[5].w = modelViewProjection[15] - modelViewProjection[14];*/

        /*
        0   =   0
        1   =   4
        2   =   8
        3   =   12
        4   =   1
        5   =   5
        6   =   9
        7   =   13
        8   =   2
        9   =   6
        10  =   10
        11  =   14
        12  =   3
        13  =   7
        14  =   11
        15  =   15
        */

        result[0].x = modelViewProjection[12] + modelViewProjection[0];
        result[0].y = modelViewProjection[13] + modelViewProjection[1];
        result[0].z = modelViewProjection[14] + modelViewProjection[2];
        result[0].w = modelViewProjection[15] + modelViewProjection[3];

        // Right Plane
        // col4 - col1
        result[1].x = modelViewProjection[12] - modelViewProjection[0];
        result[1].y = modelViewProjection[13] - modelViewProjection[1];
        result[1].z = modelViewProjection[14] - modelViewProjection[2];
        result[1].w = modelViewProjection[15] - modelViewProjection[3];

        // Bottom Plane
        // col4 + col2
        result[2].x = modelViewProjection[12] + modelViewProjection[4];
        result[2].y = modelViewProjection[13] + modelViewProjection[5];
        result[2].z = modelViewProjection[14] + modelViewProjection[6];
        result[2].w = modelViewProjection[15] + modelViewProjection[7];

        // Top Plane
        // col4 - col2
        result[3].x = modelViewProjection[12] - modelViewProjection[4];
        result[3].y = modelViewProjection[13] - modelViewProjection[5];
        result[3].z = modelViewProjection[14] - modelViewProjection[6];
        result[3].w = modelViewProjection[15] - modelViewProjection[7];

        // Near Plane
        // col4 + col3
        result[4].x = modelViewProjection[12] + modelViewProjection[8];
        result[4].y = modelViewProjection[13] + modelViewProjection[9];
        result[4].z = modelViewProjection[14] + modelViewProjection[10];
        result[4].w = modelViewProjection[15] + modelViewProjection[11];

        // Far Plane
        // col4 - col3
        result[5].x = (modelViewProjection[12] - modelViewProjection[8]);
        result[5].y = (modelViewProjection[13] - modelViewProjection[9]);
        result[5].z = (modelViewProjection[14] - modelViewProjection[10]);
        result[5].w = (modelViewProjection[15] - modelViewProjection[11]);

        if(normalize)
        {
            result[0].xyz(result[0].xyz().normalize());
            result[1].xyz(result[1].xyz().normalize());
            result[2].xyz(result[2].xyz().normalize());
            result[3].xyz(result[3].xyz().normalize());
            result[4].xyz(result[4].xyz().normalize());
            result[5].xyz(result[5].xyz().normalize());
        }

        return result;
    }
#endif

    Vector2f screen(
        Vector3f p,
        Matrix4f mvp,
        int width,
        int height)
    {
        Vector4f hp = mvp * Vector4f(p, 1.0f);
        hp /= hp.w;

        return Vector2f{
            ((hp.x + 1.0f) / 2.0f) * width,
            ((1.0f - hp.y) / 2.0f) * height };
    }

    Line clipToScreen(
        const Vector3f& a,
        const Vector3f& b,
        const Matrix4f& mvp,
        const Vector3f& cameraPosition,
        int cameraWidth,
        int cameraHeight,
        const engine::vector<Vector4f>& frustumPlanes)
    {
        Vector3f clipA = a;
        Vector3f clipB = b;
        for (auto&& plane : frustumPlanes)
        {
            float aDistance = plane.xyz().normalize().dot(clipA - cameraPosition);
            float bDistance = plane.xyz().normalize().dot(clipB - cameraPosition);

            if (aDistance < 0 && bDistance < 0)
                return Line{ {},{}, true };

            if (aDistance >= 0 && bDistance >= 0)
                continue;

            float intersectionFactor = aDistance / (aDistance - bDistance);

            Vector3f intersectionPoint{
                clipA.x + intersectionFactor * (clipB.x - clipA.x),
                clipA.y + intersectionFactor * (clipB.y - clipA.y),
                clipA.z + intersectionFactor * (clipB.z - clipA.z)
            };
            if (aDistance < 0)
                clipA = intersectionPoint;
            if (bDistance < 0)
                clipB = intersectionPoint;
        }
        return Line{
            screen(clipA, mvp, cameraWidth, cameraHeight),
            screen(clipB, mvp, cameraWidth, cameraHeight),
            false };
    }

    void drawLine3d(
        Camera& camera,
        const engine::vector<Vector4f>& frustumPlanes,
        const Matrix4f& mvp,
        ImDrawList* drawList,

        const Vector3f& a,
        const Vector3f& b,
        ImU32 color,
        float thickness)
    {
        auto clipped = clipToScreen(a, b, mvp, camera.position(), camera.width(), camera.height(), frustumPlanes);
        drawList->AddLine(
            ImVec2{ clipped.a.x, clipped.a.y },
            ImVec2{ clipped.b.x, clipped.b.y },
            color, thickness);
    }

    void drawCube3d(
        Camera& camera,
        const engine::vector<Vector4f>& frustumPlanes,
        const Matrix4f& mvp,
        ImDrawList* drawList,

        const BoundingBox& box,
        ImU32 color,
        float thickness)
    {
        engine::vector<Vector3f> corners = 
        {
            box.min,
            box.min + Vector3f{ box.width(), 0.0f, 0.0f },
            box.min + Vector3f{ box.width(), 0.0f, box.depth() },
            box.min + Vector3f{ 0.0f, 0.0f, box.depth() },

            box.min + Vector3f{ 0.0f, box.height(), 0.0f },
            box.min + Vector3f{ box.width(), box.height(), 0.0f },
            box.min + Vector3f{ box.width(), box.height(), box.depth() },
            box.min + Vector3f{ 0.0f, box.height(), box.depth() },
        };

        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[0], corners[1], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[1], corners[2], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[2], corners[3], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[3], corners[0], color, thickness);

        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[4], corners[5], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[5], corners[6], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[6], corners[7], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[7], corners[4], color, thickness);

        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[0], corners[4], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[1], corners[5], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[2], corners[6], color, thickness);
        drawLine3d(camera, frustumPlanes, mvp, drawList, corners[3], corners[7], color, thickness);
    }

    void drawCircle3d(
        Camera& camera,
        const engine::vector<Vector4f>& frustumPlanes,
        const Matrix4f& mvp,
        const Matrix4f& modelMatrix,
        ImDrawList* drawList,
        float radius,
        int angles,
        ImU32 color,
        float thickness,
        float startAngleDegrees,
        float stopAngleDegrees,
        CirclePlane circlePlane)
    {
        engine::vector<Vector4f> points(angles + 1);
        float startAngleRad = startAngleDegrees * DEG_TO_RAD;
        float stopAngleRad = stopAngleDegrees * DEG_TO_RAD;
        float angleDistanceRad = (stopAngleRad - startAngleRad);
        float angleIncrementRad = angleDistanceRad / static_cast<float>(angles);

        float currentAngle = startAngleRad;

        if (circlePlane == CirclePlane::XY)
        {
            for (int i = 0; i < angles; ++i)
            {
                Vector4f point{
                    radius * cosf(currentAngle),
                    radius * sinf(currentAngle),
                    0.0f,
                    1.0f };
                point = modelMatrix * point;
                points[i] = point;

                currentAngle += angleIncrementRad;
            }
            Vector4f point{
                radius * cosf(currentAngle),
                radius * sinf(currentAngle),
                0.0f,
                1.0f };
            point = modelMatrix * point;
            points[angles] = point;
        }
        else if (circlePlane == CirclePlane::XZ)
        {
            for (int i = 0; i < angles; ++i)
            {
                Vector4f point{
                    radius * cosf(currentAngle),
                    0.0f,
                    radius * sinf(currentAngle),
                    1.0f };
                point = modelMatrix * point;
                points[i] = point;

                currentAngle += angleIncrementRad;
            }
            Vector4f point{
                radius * cosf(currentAngle),
                0.0f,
                radius * sinf(currentAngle),
                1.0f };
            point = modelMatrix * point;
            points[angles] = point;
        }
        else if (circlePlane == CirclePlane::YZ)
        {
            for (int i = 0; i < angles; ++i)
            {
                Vector4f point{
                    0.0f,
                    radius * cosf(currentAngle),
                    radius * sinf(currentAngle),
                    1.0f };
                point = modelMatrix * point;
                points[i] = point;

                currentAngle += angleIncrementRad;
            }
            Vector4f point{
                0.0f,
                radius * cosf(currentAngle),
                radius * sinf(currentAngle),
                1.0f };
            point = modelMatrix * point;
            points[angles] = point;
        }

        for (int i = 0; i < points.size() - 1; ++i)
        {
            drawLine3d(camera, frustumPlanes, mvp, drawList, points[i].xyz(), points[i + 1].xyz(), color, thickness);
        }
    }

    GpuMarker::GpuMarker(CommandList& cmd, const char* msg)
    {
        m_api = cmd.api();
        if(m_api == GraphicsApi::DX12)
            ptr = new implementation::GpuMarkerImplDX12(cmd, msg);
        else if (m_api == GraphicsApi::Vulkan)
            ptr = new implementation::GpuMarkerImplVulkan(cmd, msg);
    }

    GpuMarker::~GpuMarker()
    {
        if (m_api == GraphicsApi::DX12)
            delete reinterpret_cast<implementation::GpuMarkerImplDX12*>(ptr);
        else if (m_api == GraphicsApi::Vulkan)
            delete reinterpret_cast<implementation::GpuMarkerImplVulkan*>(ptr);
        ptr = nullptr;
    }

    CpuMarker::CpuMarker(GraphicsApi api, const char* msg)
    {
        m_api = api;
        if (m_api == GraphicsApi::DX12)
            ptr = new implementation::CpuMarkerImplDX12(msg);
        else if (m_api == GraphicsApi::Vulkan)
            ptr = new implementation::CpuMarkerImplVulkan(msg);
    }

    CpuMarker::~CpuMarker()
    {
        if (m_api == GraphicsApi::DX12)
            delete reinterpret_cast<implementation::CpuMarkerImplDX12*>(ptr);
        else if (m_api == GraphicsApi::Vulkan)
            delete reinterpret_cast<implementation::CpuMarkerImplVulkan*>(ptr);
        ptr = nullptr;
    }
}
