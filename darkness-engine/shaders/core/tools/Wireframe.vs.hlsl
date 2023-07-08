#include "../shared_types/VertexScale.hlsli"
#include "../VertexPacking.hlsli"

Buffer<uint2> vertices;

cbuffer ConstData
{
    float4x4 jitterModelViewProjectionMatrix;
};

struct VSOutput
{
    float4 position         : SV_Position0;
    float4 color            : COLOR0;
};

static const float4 randomColors[10] =
{
    float4(0.980, 0.502, 0.447, 1.0),
    float4(0.863, 0.078, 0.235, 1.0),
    float4(1.000, 0.753, 0.796, 1.0),
    float4(1.000, 0.627, 0.478, 1.0),
    float4(1.000, 0.549, 0.000, 1.0),
    float4(1.000, 1.000, 0.000, 1.0),
    float4(1.000, 0.937, 0.835, 1.0),
    float4(0.855, 0.439, 0.839, 1.0),
    float4(0.000, 1.000, 1.000, 1.0),
    float4(0.502, 0.000, 0.502, 1.0)
};

VSOutput main(uint id : SV_VertexID)
{
    VertexScale scale;
    scale.range = float3(1.0f, 1.0f, 1.0f);
    scale.origo = float3(0.0f, 0.0f, 0.0f);

    VSOutput output;
    output.position = mul(jitterModelViewProjectionMatrix, float4(unpackVertex(vertices[id], scale), 1.0f));
    output.color = randomColors[(id / 32) % 10];
    return output;
}
