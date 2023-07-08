
#include "../Common.hlsli"

struct Transform
{
    float4x4 trans;
};

Buffer<float3> vertexes;
Buffer<float4> lightParameters;
Buffer<float> pointLightRange;
Buffer<uint> pointLightIds;
StructuredBuffer<Transform> lightTransforms;

cbuffer RenderConesConstants
{
    float4x4 jitterViewProjectionMatrix;
    uint sectors;
};

struct VSOutput
{
    float4 position     : SV_Position0;
    uint lightId		: BLENDINDICES0;
};

VSOutput main(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
    VSOutput output;
    output.position = mul(jitterViewProjectionMatrix, mul(lightTransforms[pointLightIds[instanceId]].trans, float4(vertexes[id] * pointLightRange[instanceId] * 1.1, 1.0f)));
    output.lightId = pointLightIds[instanceId];
    return output;
}
