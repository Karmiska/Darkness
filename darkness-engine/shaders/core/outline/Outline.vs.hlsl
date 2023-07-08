#include "../GBuffer.hlsli"
#include "../shared_types/VertexScale.hlsli"
#include "../VertexPacking.hlsli"

Buffer<uint2> vertices;
Buffer<float2> normals;

StructuredBuffer<VertexScale> scales;

struct VSOutput
{
    float3 position         : POSITION0;
    float3 normal           : NORMAL0;
};

cbuffer OutlineConstants
{
    uint instanceId;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.position = unpackVertex(vertices[id], scales[instanceId]);
    output.normal = unpackNormalOctahedron(normals[id]);
    return output;
}
