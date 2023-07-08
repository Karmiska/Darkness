
#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/SubMeshData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/FrustumCullingOutput.hlsli"
#include "../shared_types/LodBinding.hlsli"
#include "../shared_types/SubMeshUVLod.hlsli"
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

Texture2D<float>                        depthPyramid;
sampler                                 depthSampler;

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
    float farPlaneDistance;
    float padding0;
    float4 padding1;
    float4 padding2;
    float4 padding3;
};

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

            bool somethingVisible = false;
            qSize = (maxXY - minXY) / 4.0f;
            for (int x = 0; x < 4; ++x)
            {
                for (int y = 0; y < 4; ++y)
                {
                    float2 localMinXY = minXY + float2(qSize.x * x, qSize.y * y);
                    float2 localMaxXY = localMinXY + qSize;

                    float2 UVminXY = localMinXY / pow2size;
                    float2 UVmaxXY = localMaxXY / pow2size;

                    float biggerDimension = max(qSize.x, qSize.y);
                    uint mip = log2(biggerDimension) + 1;

                    float4 blockDepths = float4(
                        depthPyramid.SampleLevel(depthSampler, float2(UVminXY.x, UVminXY.y), mip),
                        depthPyramid.SampleLevel(depthSampler, float2(UVmaxXY.x, UVminXY.y), mip),
                        depthPyramid.SampleLevel(depthSampler, float2(UVminXY.x, UVmaxXY.y), mip),
                        depthPyramid.SampleLevel(depthSampler, float2(UVmaxXY.x, UVmaxXY.y), mip)
                        );

                    float minBlockDepth = min(min(blockDepths.x, blockDepths.y), min(blockDepths.z, blockDepths.w));

                    if (clusterMaxDepth >= minBlockDepth)
                    {
                        somethingVisible = true;
                        break;
                    }
                }
            }

            if (somethingVisible)
            {
                SubMeshData sMeshData = subMeshData[subMeshUvLod.submeshPointer];

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
        }
    }
}
