
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/ClusterData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../shared_types/SubMeshUVLod.hlsli"
#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/VertexScale.hlsli"
#include "../GBuffer.hlsli"
#include "../VertexPacking.hlsli"
#include "../Common.hlsli"

Buffer<uint2> vertices;
Buffer<float2> normals;
Buffer<float2> tangents;
Buffer<float2> uv;

StructuredBuffer<VertexScale> scales;

StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;

struct RootConstants
{
    uint clusterStart;
    uint unused1;
    uint unused2;
    uint unused3;
};
ConstantBuffer<RootConstants> rootConstants;

cbuffer CameraMatrixes
{
    float4x4 viewProjectionMatrix;
    float4x4 previousViewProjectionMatrix;
    float4x4 jitterViewProjectionMatrix;
    float3 cameraPosition;
};

struct VSOutput
{
    float4 position         : SV_Position0;
    float4 mvPosCurrent     : POSITION0;
    float4 mvPosPrevious    : POSITION1;
    float4 normal           : NORMAL0;
    float4 tangent          : TEXCOORD0;
    uint instancePointer    : BLENDINDICES0;
#ifdef OPTION_DEBUG
    uint clusterPointer     : BLENDINDICES1;
#endif
};

VSOutput main(uint id : SV_VertexID)
{
    const float4 invalid = float4(
        asfloat(0x7f800001),
        asfloat(0x7f800001),
        asfloat(0x7f800001),
        asfloat(0x7f800001));

    VSOutput output;
    if (id != 0xffffffff)
    {

        uint clusterId = (id & 0xffff0000) >> 16;
        uint clusterIndex = id & 0x0000ffff;
        ClusterInstanceData cInstanceData = clusters[rootConstants.clusterStart + clusterId];
        ClusterData cdata = clusterData[cInstanceData.clusterPointer];

        uint vertexDataIndex = cdata.vertexPointer + clusterIndex;
        uint uvDataIndex = cdata.vertexPointer + clusterIndex;

        TransformHistory transforms = transformHistory[cInstanceData.instancePointer];
        float4x4 normalTransform = transpose(transforms.inverseTransform);

        float3 vertex = unpackVertex(vertices[vertexDataIndex], scales[cInstanceData.instancePointer]);

        output.position = mul(mul(jitterViewProjectionMatrix, transforms.transform), float4(vertex, 1));
        output.mvPosCurrent = mul(mul(viewProjectionMatrix, transforms.transform), float4(vertex, 1));
        output.mvPosPrevious = mul(mul(previousViewProjectionMatrix, transforms.previousTransform), float4(vertex, 1));
        output.normal = normalize(mul(normalTransform, float4(unpackNormalOctahedron(normals[vertexDataIndex]), 0.0f)));
        output.tangent = normalize(mul(normalTransform, float4(unpackNormalOctahedron(tangents[vertexDataIndex]), 0.0f)));
        output.instancePointer = cInstanceData.instancePointer;
#ifdef OPTION_DEBUG
        output.clusterPointer = cInstanceData.clusterPointer;
#endif

        float2 uu = uv[uvDataIndex];
        output.normal.w = uu.x;
        output.tangent.w = uu.y;
        
        return output;

    }

    output.position = invalid;
    output.mvPosCurrent = invalid;
    output.mvPosPrevious = invalid;
    output.normal = invalid;
    output.tangent = invalid;
    output.instancePointer = 0;
    return output;

}
