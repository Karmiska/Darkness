#include "../shared_types/VertexScale.hlsli"
#include "../VertexPacking.hlsli"

Buffer<uint2> vertices;
Buffer<float2> uv;
Buffer<float> scales;

cbuffer ConstData
{
    float4x4 jitterModelViewProjectionMatrix;
};

struct VSOutput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

VSOutput main(uint id : SV_VertexID)
{
    VertexScale scale;
    scale.range = float3(1.0f, 1.0f, 1.0f);
    scale.origo = float3(0.0f, 0.0f, 0.0f);

    VSOutput output;
    output.position = mul(jitterModelViewProjectionMatrix, float4(unpackVertex(vertices[id], scale), 1.0f));
    output.uv = uv[id];
    return output;
}
