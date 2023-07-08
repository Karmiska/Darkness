
#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/SubMeshData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/FrustumCullingOutput.hlsli"
#include "../shared_types/LodBinding.hlsli"
#include "../shared_types/SubMeshUVLod.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../Common.hlsli"
#include "CullingFunctions.hlsli"

StructuredBuffer<BoundingBox> subMeshBoundingBoxes;
StructuredBuffer<SubMeshUVLod> lodBinding;
StructuredBuffer<LodBinding> instanceLodBinding;
StructuredBuffer<SubMeshData> subMeshData;
StructuredBuffer<TransformHistory> transformHistory;

RWStructuredBuffer<FrustumCullingOutput> cullingOutput;
RWBuffer<uint> clusterCountBuffer;
RWBuffer<uint> instanceCountBuffer;
RWBuffer<uint> outputAllocationShared;
RWBuffer<uint> outputAllocationSharedSingle;

RWStructuredBuffer<FrustumCullingOutput> cullingOutputAlphaClip;
RWBuffer<uint> clusterCountBufferAlphaClip;
RWBuffer<uint> instanceCountBufferAlphaClip;
RWBuffer<uint> outputAllocationSharedAlphaClip;
RWBuffer<uint> outputAllocationSharedAlphaClipSingle;

RWStructuredBuffer<FrustumCullingOutput> cullingOutputTerrain;
RWBuffer<uint> clusterCountBufferTerrain;
RWBuffer<uint> instanceCountBufferTerrain;
RWBuffer<uint> outputAllocationSharedTerrain;
RWBuffer<uint> outputAllocationSharedTerrainSingle;

StructuredBuffer<InstanceMaterial>      instanceMaterials;

StructuredBuffer<BoundingBox> sceneChange;
Buffer<uint> changedCounter;
RWBuffer<uint> matches;

cbuffer CullingConstants
{
    // 16 floats
    float4 plane0;
    float4 plane1;
    float4 plane2;
    float4 plane3;

    // matrices
    float4x4 viewMatrix;
    float4x4 projectionMatrix;

    // 16 floats
    float4 plane4;
    float4 plane5;

    float3 cameraPosition;
    uint instanceCount;

    float2 size;
    float2 inverseSize;

    // 16 floats
    float2 pow2size;
    float  range;
    float  farPlaneDistance;

    uint force;
    uint3 padding0;

    float4 padding1;
    float4 padding2;
    float4 padding3;
};

bool intersectBoundingBox(BoundingBox a, BoundingBox b)
{
    /*float3 sizeA = a.max - a.min;
    float3 centerA = a.min + (sizeA / 2);

    float3 sizeB = b.max - b.min;
    float3 centerB = b.min + (sizeB / 2);

    float3 T = centerB - centerA;

    return 
        abs(T.x) <= (sizeA.x + sizeB.x) &&
        abs(T.y) <= (sizeA.y + sizeB.y) &&
        abs(T.z) <= (sizeA.z + sizeB.z);*/

    return 
        (a.min.x < b.max.x) && 
        (a.max.x > b.min.x) &&
        (a.min.y < b.max.y) && 
        (a.max.y > b.min.y) &&
        (a.min.z < b.max.z) && 
        (a.max.z > b.min.z);
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
        TransformHistory transforms = transformHistory[instanceId];
        LodBinding lodBind = instanceLodBinding[instanceId];

        // we pick the base lod to get a bounding box
        SubMeshUVLod subMeshUvLod = lodBinding[lodBind.lodPointer];
        BoundingBox bb = subMeshBoundingBoxes[subMeshUvLod.submeshPointer];

        float2 minXY;
        float2 maxXY;
        float clusterMaxDepth;
        screenMinMaxXYZ(
            viewMatrix, projectionMatrix, transforms.transform,
            bb.min, bb.max, size,
            minXY, maxXY,
            clusterMaxDepth);

        float2 qSize = maxXY - minXY;
        uint selectedLod = lodFromScreenSize(qSize);
        uint lodIndex = (min(selectedLod, lodBind.lodCount - 1));

        // now we get the actual LOD data
        subMeshUvLod = lodBinding[lodBind.lodPointer + lodIndex];
        bb = subMeshBoundingBoxes[subMeshUvLod.submeshPointer];

        if (frustumCull(
            bb.min, bb.max,
            transforms.transform,
            cameraPosition,
            farPlaneDistance,
            plane0.xyz,
            plane1.xyz,
            plane2.xyz,
            plane3.xyz,
            plane4.xyz,
            plane5.xyz))
        {
            /*float3 cornerCurrent[8] =
            {
                mul(transforms.transform, float4(bb.min.xyz, 1.0f)).xyz,
                mul(transforms.transform, float4(bb.min.x, bb.max.y, bb.max.z, 1.0f)).xyz,
                mul(transforms.transform, float4(bb.min.x, bb.min.y, bb.max.z, 1.0f)).xyz,
                mul(transforms.transform, float4(bb.min.x, bb.max.y, bb.min.z, 1.0f)).xyz,
                mul(transforms.transform, float4(bb.max, 1.0f)).xyz,
                mul(transforms.transform, float4(bb.max.x, bb.min.y, bb.min.z, 1.0f)).xyz,
                mul(transforms.transform, float4(bb.max.x, bb.max.y, bb.min.z, 1.0f)).xyz,
                mul(transforms.transform, float4(bb.max.x, bb.min.y, bb.max.z, 1.0f)).xyz
            };

            BoundingBox transformedBB;
            transformedBB.min = float3(
                minValueX(cornerCurrent),
                minValueY(cornerCurrent),
                minValueZ(cornerCurrent));
            transformedBB.max = float3(
                maxValueX(cornerCurrent),
                maxValueY(cornerCurrent),
                maxValueZ(cornerCurrent));*/

            bool found = force > 0;
            if (!found)
            {
                for (int i = 0; i < changedCounter[0]; ++i)
                {
                    /*if (intersectBoundingBox(transformedBB, sceneChange[i]))
                    {
                        found = true;
                        break;
                    }*/
                    if (frustumCullNoTransform(
                        sceneChange[i].min, sceneChange[i].max,
                        cameraPosition,
                        farPlaneDistance,
                        plane0.xyz,
                        plane1.xyz,
                        plane2.xyz,
                        plane3.xyz,
                        plane4.xyz,
                        plane5.xyz))
                    {
                        found = true;
                        break;
                    }
                }
            }

            if (found)
            {
                SubMeshData sMeshData = subMeshData[subMeshUvLod.submeshPointer];

                InstanceMaterial material = instanceMaterials[instanceId];
                if ((material.materialSet & 0x20) == 0x20)     // alphaclipped
                {
                    uint newLocation;
                    uint newLocationSingle;

                    InterlockedAdd(outputAllocationSharedAlphaClip[0], sMeshData.clusterCount, newLocation);
                    InterlockedAdd(outputAllocationSharedAlphaClipSingle[0], 1, newLocationSingle);

                    cullingOutputAlphaClip[newLocationSingle].outputPointer = newLocation;
                    cullingOutputAlphaClip[newLocationSingle].clusterPointer = sMeshData.clusterPointer;
                    cullingOutputAlphaClip[newLocationSingle].clusterCount = sMeshData.clusterCount;
                    cullingOutputAlphaClip[newLocationSingle].instancePointer = instanceId;
                    cullingOutputAlphaClip[newLocationSingle].uvPointer = subMeshUvLod.uvPointer;
                    cullingOutputAlphaClip[newLocationSingle].lodPointer = lodBind.lodPointer + lodIndex;
                    cullingOutputAlphaClip[newLocationSingle].padding1 = 0;
                    cullingOutputAlphaClip[newLocationSingle].padding2 = 0;

                    InterlockedAdd(clusterCountBufferAlphaClip[0], sMeshData.clusterCount);
                    InterlockedAdd(instanceCountBufferAlphaClip[0], 1);
                }
                else if ((material.materialSet & 0x40) == 0x40)     // transparent
                {
                    // we don't emit these
                }
				else if ((material.materialSet & 0x80) == 0x80)     // terrain
				{
					uint newLocation;
                    uint newLocationSingle;

					InterlockedAdd(outputAllocationSharedTerrain[0], sMeshData.clusterCount, newLocation);
                    InterlockedAdd(outputAllocationSharedTerrainSingle[0], 1, newLocationSingle);

					cullingOutputTerrain[newLocationSingle].outputPointer = newLocation;
                    cullingOutputTerrain[newLocationSingle].clusterPointer = sMeshData.clusterPointer;
                    cullingOutputTerrain[newLocationSingle].clusterCount = sMeshData.clusterCount;
                    cullingOutputTerrain[newLocationSingle].instancePointer = instanceId;
                    cullingOutputTerrain[newLocationSingle].uvPointer = subMeshUvLod.uvPointer;
                    cullingOutputTerrain[newLocationSingle].lodPointer = lodBind.lodPointer + lodIndex;
                    cullingOutputTerrain[newLocationSingle].padding1 = 0;
                    cullingOutputTerrain[newLocationSingle].padding2 = 0;

					InterlockedAdd(clusterCountBufferTerrain[0], sMeshData.clusterCount);
					InterlockedAdd(instanceCountBufferTerrain[0], 1);
				}
                else
                {
                    uint newLocation;
                    uint newLocationSingle;

                    InterlockedAdd(outputAllocationShared[0], sMeshData.clusterCount, newLocation);
                    InterlockedAdd(outputAllocationSharedSingle[0], 1, newLocationSingle);

                    cullingOutput[newLocationSingle].outputPointer = newLocation;
                    cullingOutput[newLocationSingle].clusterPointer = sMeshData.clusterPointer;
                    cullingOutput[newLocationSingle].clusterCount = sMeshData.clusterCount;
                    cullingOutput[newLocationSingle].instancePointer = instanceId;
                    cullingOutput[newLocationSingle].uvPointer = subMeshUvLod.uvPointer;
                    cullingOutput[newLocationSingle].lodPointer = lodBind.lodPointer + lodIndex;
                    cullingOutput[newLocationSingle].padding1 = 0;
                    cullingOutput[newLocationSingle].padding2 = 0;

                    InterlockedAdd(clusterCountBuffer[0], sMeshData.clusterCount);
                    InterlockedAdd(instanceCountBuffer[0], 1);
                }

                uint newCount = 0;
                InterlockedAdd(matches[0], 1, newCount);
            }
        }
    }
}
