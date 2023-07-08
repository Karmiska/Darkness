
#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/SubMeshData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/LodBinding.hlsli"
#include "../shared_types/SubMeshUVLod.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../Common.hlsli"
#include "CullingFunctions.hlsli"

StructuredBuffer<ClusterInstanceData>   inputClusters;
Buffer<uint>                            inputClusterIndex;
Buffer<uint>                            inputClusterCount;

RWStructuredBuffer<ClusterInstanceData> outputClusters;
Buffer<uint>                            outputClusterIndex;
RWBuffer<uint>                          outputClusterDistributor;

RWStructuredBuffer<ClusterInstanceData> alphaClippedOutputClusters;
Buffer<uint>                            alphaClippedOutputClusterIndex;
RWBuffer<uint>                          alphaClippedOutputClusterDistributor;

RWStructuredBuffer<ClusterInstanceData> transparentOutputClusters;
Buffer<uint>                            transparentOutputClusterIndex;
RWBuffer<uint>                          transparentOutputClusterDistributor;

RWStructuredBuffer<ClusterInstanceData> terrainOutputClusters;
Buffer<uint>                            terrainOutputClusterIndex;
RWBuffer<uint>                          terrainOutputClusterDistributor;

StructuredBuffer<SubMeshData>           subMeshData;
StructuredBuffer<BoundingBox>           subMeshBoundingBoxes;
StructuredBuffer<SubMeshUVLod>          lodBinding;
StructuredBuffer<LodBinding>            instanceLodBinding;
StructuredBuffer<TransformHistory>      transformHistory;

Buffer<uint>                            clusterAccurateTrackingInstancePtr;
RWBuffer<uint>                          clusterAccurateTracking;

StructuredBuffer<InstanceMaterial>      instanceMaterials;

cbuffer CullingConstants
{
    // 16 floats
    float4      plane0;
    float4      plane1;
    float4      plane2;
    float4      plane3;

    // matrices
    float4x4    viewMatrix;
    float4x4    projectionMatrix;

    // 16 floats
    float4      plane4;
    float4      plane5;
    float3      cameraPosition;
    uint        instanceCount;
    float2      size;
    float2      inverseSize;

    // 16 floats
    uint        fnvPrime;
    uint        fnvOffsetBasis;
    float       farPlaneDistance;
    uint        padding0;
    float4      padding2;
    float4      padding3;
    float4      padding4;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < inputClusterCount[0])
    {
        ClusterInstanceData cInstanceData = inputClusters[inputClusterIndex[0] + dispatchThreadID.x];

        uint instanceId = cInstanceData.instancePointer;

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
        SubMeshUVLod subMeshUvLodReal = lodBinding[lodBind.lodPointer + lodIndex];
        bb = subMeshBoundingBoxes[subMeshUvLodReal.submeshPointer];

        if ((lodBind.lodPointer + lodIndex == cInstanceData.lodPointer) &&
            frustumCull(
            bb.min, bb.max,
            transforms.transform,
            cameraPosition, farPlaneDistance,
            plane0.xyz, plane1.xyz, plane2.xyz, plane3.xyz, plane4.xyz, plane5.xyz))
        {

            uint clusterTrackingInstanceId = clusterAccurateTrackingInstancePtr[instanceId];
            uint zeroClusterPtr = subMeshData[subMeshUvLod.submeshPointer].clusterPointer;
            uint clusterDelta = cInstanceData.clusterPointer - zeroClusterPtr;
            uint binLocation = (clusterTrackingInstanceId + clusterDelta) / 32;
            uint bitLocation = (clusterTrackingInstanceId + clusterDelta) - (binLocation * 32);
            uint originalMask;
            InterlockedOr(clusterAccurateTracking[binLocation], (uint)1 << bitLocation, originalMask);

            if((originalMask & ((uint)1 << bitLocation)) == 0)
            {
                InstanceMaterial material = instanceMaterials[cInstanceData.instancePointer];
                if ((material.materialSet & 0x20) == 0x20)     // alphaclipped
                {
                    uint newIndex = 0;
                    InterlockedAdd(alphaClippedOutputClusterDistributor[0], 1, newIndex);
                    alphaClippedOutputClusters[alphaClippedOutputClusterIndex[0] + newIndex] = cInstanceData;
                }
                else if ((material.materialSet & 0x40) == 0x40)     // transparent
                {
                    uint newIndex = 0;
                    InterlockedAdd(transparentOutputClusterDistributor[0], 1, newIndex);
                    transparentOutputClusters[transparentOutputClusterIndex[0] + newIndex] = cInstanceData;
                }
				else if ((material.materialSet & 0x80) == 0x80)     // terrain
				{
					uint newIndex = 0;
					InterlockedAdd(terrainOutputClusterDistributor[0], 1, newIndex);
					terrainOutputClusters[terrainOutputClusterIndex[0] + newIndex] = cInstanceData;
				}
                else
                {
                    uint newIndex = 0;
                    InterlockedAdd(outputClusterDistributor[0], 1, newIndex);
                    outputClusters[outputClusterIndex[0] + newIndex] = cInstanceData;
                }

            }
        }
    }

}
