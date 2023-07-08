#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/SubMeshData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/FrustumCullingOutput.hlsli"
#include "../shared_types/LodBinding.hlsli"
#include "../shared_types/SubMeshUVLod.hlsli"
#include "../Common.hlsli"

StructuredBuffer<BoundingBox> subMeshBoundingBoxes;
StructuredBuffer<SubMeshUVLod> lodBinding;
StructuredBuffer<LodBinding> instanceLodBinding;
StructuredBuffer<TransformHistory> transformHistory;
RWStructuredBuffer<BoundingBox> sceneChange;
RWBuffer<uint> changedCounter;

cbuffer CullingConstants
{
    uint instanceCount;
};

bool transformChanged(TransformHistory transform, uint instanceId)
{
    uint change = 0;
    change |= (uint)(transform.previousTransform._m00 != transform.transform._m00);
    change |= (uint)(transform.previousTransform._m01 != transform.transform._m01);
    change |= (uint)(transform.previousTransform._m02 != transform.transform._m02);
    change |= (uint)(transform.previousTransform._m03 != transform.transform._m03);
    change |= (uint)(transform.previousTransform._m10 != transform.transform._m10);
    change |= (uint)(transform.previousTransform._m11 != transform.transform._m11);
    change |= (uint)(transform.previousTransform._m12 != transform.transform._m12);
    change |= (uint)(transform.previousTransform._m13 != transform.transform._m13);
    change |= (uint)(transform.previousTransform._m20 != transform.transform._m20);
    change |= (uint)(transform.previousTransform._m21 != transform.transform._m21);
    change |= (uint)(transform.previousTransform._m22 != transform.transform._m22);
    change |= (uint)(transform.previousTransform._m23 != transform.transform._m23);
    change |= (uint)(transform.previousTransform._m30 != transform.transform._m30);
    change |= (uint)(transform.previousTransform._m31 != transform.transform._m31);
    change |= (uint)(transform.previousTransform._m32 != transform.transform._m32);
    change |= (uint)(transform.previousTransform._m33 != transform.transform._m33);
    return change;
}

float minValueX(float3 vec[8])
{
    return min(
        min(min(vec[0].x, min(vec[1].x, vec[2].x)), vec[3].x), 
        min(min(vec[4].x, min(vec[5].x, vec[6].x)), vec[7].x));
}

float minValueY(float3 vec[8])
{
    return min(
        min(min(vec[0].y, min(vec[1].y, vec[2].y)), vec[3].y),
        min(min(vec[4].y, min(vec[5].y, vec[6].y)), vec[7].y));
}

float minValueZ(float3 vec[8])
{
    return min(
        min(min(vec[0].z, min(vec[1].z, vec[2].z)), vec[3].z),
        min(min(vec[4].z, min(vec[5].z, vec[6].z)), vec[7].z));
}

float maxValueX(float3 vec[8])
{
    return max(
        max(max(vec[0].x, max(vec[1].x, vec[2].x)), vec[3].x),
        max(max(vec[4].x, max(vec[5].x, vec[6].x)), vec[7].x));
}

float maxValueY(float3 vec[8])
{
    return max(
        max(max(vec[0].y, max(vec[1].y, vec[2].y)), vec[3].y),
        max(max(vec[4].y, max(vec[5].y, vec[6].y)), vec[7].y));
}

float maxValueZ(float3 vec[8])
{
    return max(
        max(max(vec[0].z, max(vec[1].z, vec[2].z)), vec[3].z),
        max(max(vec[4].z, max(vec[5].z, vec[6].z)), vec[7].z));
}

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint instanceId = dispatchThreadID.x;
    if (instanceId < instanceCount)
    {
        TransformHistory transform = transformHistory[instanceId];

        if (transformChanged(transform, instanceId))
        {
            LodBinding lodBind = instanceLodBinding[instanceId];

            // we pick the base lod to get a bounding box
            SubMeshUVLod subMeshUvLod = lodBinding[lodBind.lodPointer];
            BoundingBox bb = subMeshBoundingBoxes[subMeshUvLod.submeshPointer];

            float3 cornerCurrent[8] =
            {
                mul(transform.transform, float4(bb.min.xyz, 1.0f)).xyz,
                mul(transform.transform, float4(bb.min.x, bb.max.y, bb.max.z, 1.0f)).xyz,
                mul(transform.transform, float4(bb.min.x, bb.min.y, bb.max.z, 1.0f)).xyz,
                mul(transform.transform, float4(bb.min.x, bb.max.y, bb.min.z, 1.0f)).xyz,
                mul(transform.transform, float4(bb.max, 1.0f)).xyz,
                mul(transform.transform, float4(bb.max.x, bb.min.y, bb.min.z, 1.0f)).xyz,
                mul(transform.transform, float4(bb.max.x, bb.max.y, bb.min.z, 1.0f)).xyz,
                mul(transform.transform, float4(bb.max.x, bb.min.y, bb.max.z, 1.0f)).xyz
            };

            float3 cornerPrev[8] =
            {
                mul(transform.previousTransform, float4(bb.min.xyz, 1.0f)).xyz,
                mul(transform.previousTransform, float4(bb.min.x, bb.max.y, bb.max.z, 1.0f)).xyz,
                mul(transform.previousTransform, float4(bb.min.x, bb.min.y, bb.max.z, 1.0f)).xyz,
                mul(transform.previousTransform, float4(bb.min.x, bb.max.y, bb.min.z, 1.0f)).xyz,
                mul(transform.previousTransform, float4(bb.max, 1.0f)).xyz,
                mul(transform.previousTransform, float4(bb.max.x, bb.min.y, bb.min.z, 1.0f)).xyz,
                mul(transform.previousTransform, float4(bb.max.x, bb.max.y, bb.min.z, 1.0f)).xyz,
                mul(transform.previousTransform, float4(bb.max.x, bb.min.y, bb.max.z, 1.0f)).xyz
            };

            uint nnindex = 0;
            InterlockedAdd(changedCounter[0], 1, nnindex);

            //sceneChange[nnindex].min = float3(-100, -100, -100);
            //sceneChange[nnindex].max = float3(100, 100, 100);

            sceneChange[nnindex * 2].min = float3(
                min(minValueX(cornerCurrent), minValueX(cornerPrev)),
                min(minValueY(cornerCurrent), minValueY(cornerPrev)),
                min(minValueZ(cornerCurrent), minValueZ(cornerPrev)));
            sceneChange[nnindex * 2].max = float3(
                max(maxValueX(cornerCurrent), maxValueX(cornerPrev)),
                max(maxValueY(cornerCurrent), maxValueY(cornerPrev)),
                max(maxValueZ(cornerCurrent), maxValueZ(cornerPrev)));
        }
    }
}
