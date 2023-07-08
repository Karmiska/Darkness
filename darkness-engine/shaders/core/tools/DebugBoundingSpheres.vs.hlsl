
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/ClusterData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/BoundingBox.hlsli"

static const float4 vertices[36] =
{
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),

    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),
    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),

    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),

    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),

    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),

    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f)
};

static const float2 uv[36] =
{
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
                      
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
                      
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
                      
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),

    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),

    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f,0.0f),
    float2(1.0f,0.0f),
    float2(1.0f, 1.0f)
};

static const float4 normals[36] =
{
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4( 0.0f,  0.0f, -1.0f, 0.0f),
    float4( 0.0f,  0.0f, -1.0f, 0.0f),
    float4( 0.0f,  0.0f, -1.0f, 0.0f),

    float4( 0.0f, -1.0f,  0.0f, 0.0f),
    float4( 0.0f, -1.0f,  0.0f, 0.0f),
    float4( 0.0f, -1.0f,  0.0f, 0.0f),
    float4( 0.0f,  0.0f, -1.0f, 0.0f),
    float4( 0.0f,  0.0f, -1.0f, 0.0f),
    float4( 0.0f,  0.0f, -1.0f, 0.0f),

    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4( 0.0f, -1.0f,  0.0f, 0.0f),
    float4( 0.0f, -1.0f,  0.0f, 0.0f),
    float4( 0.0f, -1.0f,  0.0f, 0.0f),

    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f),

    float4(1.0f,  0.0f,  0.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f),
    float4(0.0f,  1.0f,  0.0f, 0.0f),
    float4(0.0f,  1.0f,  0.0f, 0.0f),
    float4(0.0f,  1.0f,  0.0f, 0.0f),

    float4(0.0f,  1.0f,  0.0f, 0.0f),
    float4(0.0f,  1.0f,  0.0f, 0.0f),
    float4(0.0f,  1.0f,  0.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f)
};

static const float4 tangents[36] =
{
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(-1.0f,  0.0f, 0.0f, 0.0f),
    float4(-1.0f,  0.0f, 0.0f, 0.0f),
    float4(-1.0f,  0.0f, 0.0f, 0.0f),

    float4(1.0f, 0.0f,  0.0f, 0.0f),
    float4(1.0f, 0.0f,  0.0f, 0.0f),
    float4(1.0f, 0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f, 0.0f, 0.0f),
    float4(-1.0f,  0.0f, 0.0f, 0.0f),
    float4(-1.0f,  0.0f, 0.0f, 0.0f),

    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f,  1.0f, 0.0f),
    float4(1.0f, 0.0f,  0.0f, 0.0f),
    float4(1.0f, 0.0f,  0.0f, 0.0f),
    float4(1.0f, 0.0f,  0.0f, 0.0f),

    float4(1.0f,  0.0f,  1.0f, 0.0f),
    float4(1.0f,  0.0f,  1.0f, 0.0f),
    float4(1.0f,  0.0f,  1.0f, 0.0f),
    float4(0.0f,  0.0f, -1.0f, 0.0f),
    float4(0.0f,  0.0f, -1.0f, 0.0f),
    float4(0.0f,  0.0f, -1.0f, 0.0f),

    float4(0.0f,  0.0f, -1.0f, 0.0f),
    float4(0.0f,  0.0f, -1.0f, 0.0f),
    float4(0.0f,  0.0f, -1.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),

    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(-1.0f,  0.0f,  0.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f),
    float4(1.0f,  0.0f,  0.0f, 0.0f)
};

StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;
StructuredBuffer<BoundingBox> boundingBoxes;

cbuffer ConstData
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;

    float4x4 previousViewMatrix;
    float4x4 previousProjectionMatrix;

    float4x4 jitterMatrix;
};

struct VSOutput
{
    float4 position         : SV_Position0;
    float4 mvPosCurrent     : POSITION0;
    float4 mvPosPrevious    : POSITION1;
    float4 normal           : NORMAL0;
    float4 tangent          : TEXCOORD0;
    float2 uv               : TEXCOORD1;
    uint instanceId         : BLENDINDICES0;
};

VSOutput main(uint id : SV_VertexID)
{
    uint clusterId = id / (3 * 64);
    uint indexId = id - (clusterId * (3 * 64));
    ClusterInstanceData cInstanceData = clusters[clusterId];
    uint instanceId = cInstanceData.instancePointer;
    ClusterData cdata = clusterData[cInstanceData.clusterPointer];

    TransformHistory transforms = transformHistory[instanceId];

    float4x4 modelMatrix = transforms.transform;
    float4x4 cameraTransform = mul(viewMatrix, modelMatrix);
    float4x4 modelViewProjectionMatrix = mul(projectionMatrix, cameraTransform);
    float4x4 previousModelViewProjectionMatrix = mul(previousProjectionMatrix, mul(previousViewMatrix, transforms.transform)); // transforms.previousTransform
    float4x4 jitterModelViewProjectionMatrix = mul(jitterMatrix, mul(projectionMatrix, cameraTransform));

    //float4 bsphere = boundingSpheres[binding.clusterId];
    float3 bbmin = boundingBoxes[cInstanceData.clusterPointer].min;
    float3 bbmax = boundingBoxes[cInstanceData.clusterPointer].max;

    float4 bsvertices[36] =
    {
        float4(bbmin.x, bbmin.y, bbmin.z, 1.0f),
        float4(bbmin.x, bbmin.y, bbmax.z, 1.0f),
        float4(bbmin.x, bbmax.y, bbmax.z, 1.0f),
        float4(bbmax.x, bbmax.y, bbmin.z,  1.0f),
        float4(bbmin.x, bbmin.y, bbmin.z, 1.0f),
        float4(bbmin.x, bbmax.y, bbmin.z, 1.0f),

        float4(bbmax.x, bbmin.y, bbmax.z,  1.0f),
        float4(bbmin.x, bbmin.y, bbmin.z, 1.0f),
        float4(bbmax.x, bbmin.y, bbmin.z,  1.0f),
        float4(bbmax.x, bbmax.y, bbmin.z,  1.0f),
        float4(bbmax.x, bbmin.y, bbmin.z,  1.0f),
        float4(bbmin.x, bbmin.y, bbmin.z, 1.0f),

        float4(bbmin.x, bbmin.y, bbmin.z, 1.0f),
        float4(bbmin.x, bbmax.y, bbmax.z, 1.0f),
        float4(bbmin.x, bbmax.y, bbmin.z, 1.0f),
        float4(bbmax.x, bbmin.y, bbmax.z,  1.0f),
        float4(bbmin.x, bbmin.y, bbmax.z, 1.0f),
        float4(bbmin.x, bbmin.y, bbmin.z, 1.0f),

        float4(bbmin.x, bbmax.y, bbmax.z, 1.0f),
        float4(bbmin.x, bbmin.y, bbmax.z, 1.0f),
        float4(bbmax.x, bbmin.y, bbmax.z,  1.0f),
        float4(bbmax.x, bbmax.y, bbmax.z,  1.0f),
        float4(bbmax.x, bbmin.y, bbmin.z,  1.0f),
        float4(bbmax.x, bbmax.y, bbmin.z,  1.0f),

        float4(bbmax.x, bbmin.y, bbmin.z,  1.0f),
        float4(bbmax.x, bbmax.y, bbmax.z,  1.0f),
        float4(bbmax.x, bbmin.y, bbmax.z,  1.0f),
        float4(bbmax.x, bbmax.y, bbmax.z,  1.0f),
        float4(bbmax.x, bbmax.y, bbmin.z,  1.0f),
        float4(bbmin.x, bbmax.y, bbmin.z, 1.0f),

        float4(bbmax.x, bbmax.y, bbmax.z,  1.0f),
        float4(bbmin.x, bbmax.y, bbmin.z, 1.0f),
        float4(bbmin.x, bbmax.y, bbmax.z, 1.0f),
        float4(bbmax.x, bbmax.y, bbmax.z,  1.0f),
        float4(bbmin.x, bbmax.y, bbmax.z, 1.0f),
        float4(bbmax.x, bbmin.y, bbmax.z,  1.0f)
    };


    VSOutput output;
    output.position = mul(jitterModelViewProjectionMatrix, bsvertices[indexId]);
    output.mvPosCurrent = mul(modelViewProjectionMatrix, bsvertices[indexId]);
    output.mvPosPrevious = mul(previousModelViewProjectionMatrix, bsvertices[indexId]);

    output.normal = mul(modelMatrix, normals[indexId]);
    output.tangent = mul(modelMatrix, tangents[indexId]);
    output.uv = uv[indexId];
    output.instanceId = instanceId;

    return output;
}
