
static const float2 vertices[6] =
{
    float2(1.0, 1.0),
    float2(0.0, 1.0),
    float2(0.0, 0.0),

    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(1.0, 1.0)
};

cbuffer ConstData
{
    uint slices;
    uint mips;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 uv : TEXCOORD0;
    float mip : PSIZE0;
    uint slice : BLENDINDICES0;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.uv = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.mip = 0;
    output.slice = 0;
    return output;
}
