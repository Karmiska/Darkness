
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/ClusterData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/VertexScale.hlsli"
#include "../VertexPacking.hlsli"

Buffer<uint2> vertices;
Buffer<uint> indexes;

StructuredBuffer<VertexScale> scales;

StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;
Buffer<uint> clusterMap;

cbuffer ConstData
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 jitterMatrix;
};

float4 main(uint id : SV_VertexID) : SV_Position
{
    uint clusterId = id / (3 * 64);
    uint indexId = id - (clusterId * (3 * 64));
    ClusterInstanceData cInstanceData = clusters[clusterId];
    ClusterData cdata = clusterData[cInstanceData.clusterPointer];

    uint index = indexes[cdata.indexPointer + indexId];
    TransformHistory transforms = transformHistory[cInstanceData.instancePointer];

    float4x4 modelMatrix = transforms.transform;
    float4x4 cameraTransform = mul(viewMatrix, modelMatrix);
    float4x4 jitterModelViewProjectionMatrix = mul(jitterMatrix, mul(projectionMatrix, cameraTransform));

    return mul(jitterModelViewProjectionMatrix, float4(unpackVertex(vertices[index], scales[cInstanceData.instancePointer]), 1.0f));
}
