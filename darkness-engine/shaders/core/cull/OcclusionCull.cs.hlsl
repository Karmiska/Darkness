
#include "../Common.hlsli"
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../shared_types/LodBinding.hlsli"
#include "../shared_types/SubMeshUVLod.hlsli"
#include "../shared_types/SubMeshData.hlsli"
#include "CullingFunctions.hlsli"

Buffer<uint>                            inputClusterIndex;
Buffer<uint>                            inputClusterCount;

RWStructuredBuffer<ClusterInstanceData> outputClusters;
Buffer<uint>                            outputClusterIndex;
RWBuffer<uint>                          outputClusterDistributor;

RWStructuredBuffer<ClusterInstanceData> notDrawnOutputClusters;
RWBuffer<uint>                          notDrawnOutputClusterDistributor;

RWStructuredBuffer<ClusterInstanceData> alphaClippedOutputClusters;
Buffer<uint>                            alphaClippedOutputClusterIndex;
RWBuffer<uint>                          alphaClippedOutputClusterDistributor;

RWStructuredBuffer<ClusterInstanceData> transparentOutputClusters;
Buffer<uint>                            transparentOutputClusterIndex;
RWBuffer<uint>                          transparentOutputClusterDistributor;

RWStructuredBuffer<ClusterInstanceData> terrainOutputClusters;
Buffer<uint>                            terrainOutputClusterIndex;
RWBuffer<uint>                          terrainOutputClusterDistributor;

StructuredBuffer<BoundingBox>           boundingBoxes;
StructuredBuffer<TransformHistory>      transformHistory;
Buffer<float4>                          clusterCones;
Texture2D<float>                        depthPyramid;
sampler                                 depthSampler;

StructuredBuffer<SubMeshUVLod>          lodBinding;
StructuredBuffer<LodBinding>            instanceLodBinding;

StructuredBuffer<SubMeshData>           subMeshData;
Buffer<uint>                            clusterAccurateTrackingInstancePtr;
RWBuffer<uint>                          clusterAccurateTracking;

StructuredBuffer<InstanceMaterial>      instanceMaterials;

cbuffer CullingConstants
{
    // 16
    float4x4 viewMatrix;

    // 16
    float4x4 invViewMatrix;

    // 16
    float4x4 projectionMatrix;

    // 16
    float4 cameraPosition;

    float2 size;
    float2 inverseSize;

    float2 pow2size;
    float2 nearFar;

    float4 mipLevels;

    // 16
    float4 plane0;
    float4 plane1;
    float4 plane2;
    float4 plane3;

    // 16
    float4 plane4;
    float4 plane5;
    float4 cameraDirection;
    uint   fnvPrime;
    uint   fnvOffsetBasis;
    uint   prepassDrawTrackingSize;
    float   farPlaneDistance;
};

#ifdef OPTION_EMIT_ALL
[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{

    if (dispatchThreadID.x < inputClusterCount[0])
    {
        ClusterInstanceData cInstanceData = outputClusters[inputClusterIndex[0] + dispatchThreadID.x];

        uint newIndex = 0;
        InterlockedAdd(outputClusterDistributor[0], 1, newIndex);
        outputClusters[outputClusterIndex[0] + newIndex] = cInstanceData;
    }
}
#else

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{

    if (dispatchThreadID.x < inputClusterCount[0])
    {
        ClusterInstanceData cInstanceData = outputClusters[inputClusterIndex[0] + dispatchThreadID.x];

        TransformHistory transforms = transformHistory[cInstanceData.instancePointer];

        BoundingBox bb = boundingBoxes[cInstanceData.clusterPointer];

        if (frustumCull(bb.min, bb.max, transforms.transform, cameraPosition.xyz, farPlaneDistance,
            plane0.xyz, plane1.xyz, plane2.xyz, plane3.xyz, plane4.xyz, plane5.xyz)
			&& coneCull(transforms.transform, clusterCones[cInstanceData.clusterPointer], cameraPosition, bb.min, bb.max)
			)
        {
            float2 minXY;
            float2 maxXY;
            float clusterMaxDepth;
            screenMinMaxXYZ(
                viewMatrix, projectionMatrix, transforms.transform,
                bb.min, bb.max, size,
                minXY, maxXY,
                clusterMaxDepth);

            bool somethingVisible = false;
            float2 qSize = (maxXY - minXY) / 4.0f;
            for (int x = 0; x < 4; ++x)
            {
                for (int y = 0; y < 4; ++y)
                {
                    float2 localMinXY = minXY + float2(qSize.x * x, qSize.y * y);
                    float2 localMaxXY = localMinXY + qSize;

                    float2 UVminXY = localMinXY / pow2size;
                    float2 UVmaxXY = localMaxXY / pow2size;

                    float biggerDimension = max(qSize.x, qSize.y);
                    uint mip = min(log2(biggerDimension) + 1, mipLevels.x-1);

                    float4 blockDepths = float4(
                        depthPyramid.SampleLevel(depthSampler, float2(UVminXY.x, UVminXY.y), mip),
                        depthPyramid.SampleLevel(depthSampler, float2(UVmaxXY.x, UVminXY.y), mip),
                        depthPyramid.SampleLevel(depthSampler, float2(UVminXY.x, UVmaxXY.y), mip),
                        depthPyramid.SampleLevel(depthSampler, float2(UVmaxXY.x, UVmaxXY.y), mip));

                    float minBlockDepth = min(min(blockDepths.x, blockDepths.y), min(blockDepths.z, blockDepths.w));

                    if (clusterMaxDepth >= minBlockDepth)
                    {
                        somethingVisible = true;
                        break;
                    }
                }
				if (somethingVisible)
					break;
            }

            if (somethingVisible)
            {
                // these are ALL clusters that pass occlusion test.
                // this list will be used as basis for next frame
                uint newIndex = 0;
                InterlockedAdd(outputClusterDistributor[0], 1, newIndex);
                outputClusters[outputClusterIndex[0] + newIndex] = cInstanceData;


                LodBinding lodBind = instanceLodBinding[cInstanceData.instancePointer];
                SubMeshUVLod subMeshUvLod = lodBinding[lodBind.lodPointer];

                uint clusterTrackingInstanceId = clusterAccurateTrackingInstancePtr[cInstanceData.instancePointer];
                uint zeroClusterPtr = subMeshData[subMeshUvLod.submeshPointer].clusterPointer;
                uint clusterDelta = cInstanceData.clusterPointer - zeroClusterPtr;
                uint binLocation = (clusterTrackingInstanceId + clusterDelta) / 32;
                uint bitLocation = (clusterTrackingInstanceId + clusterDelta) - (binLocation * 32);
                bool clusterDrawnAlready = clusterAccurateTracking[binLocation] & ((uint)1 << bitLocation);

                // these are the clusters that we have not yet drawn to depth + GBuffer in the first pass
                // first pass draws only depth for alphaclipped clusters
                if (!clusterDrawnAlready)
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
                        // these are passed, not yet draw clusters
                        // waiting for full depth + GBuffer pass
                        uint nnindex = 0;
                        InterlockedAdd(notDrawnOutputClusterDistributor[0], 1, nnindex);
                        notDrawnOutputClusters[nnindex] = cInstanceData;
                    }
                }
            }
        }
    }
}
#endif
