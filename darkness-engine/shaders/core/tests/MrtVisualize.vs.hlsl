
static const float2 vertices[24] =
{
    float2(-0.1, 0.9),
    float2(-0.9, 0.9),
    float2(-0.9, 0.1),

    float2(-0.9, 0.1),
    float2(-0.1, 0.1),
    float2(-0.1, 0.9),

    float2(0.9, 0.9),
    float2(0.1, 0.9),
    float2(0.1, 0.1),

    float2(0.1, 0.1),
    float2(0.9, 0.1),
    float2(0.9, 0.9),

    float2(-0.1, -0.1),
    float2(-0.9, -0.1),
    float2(-0.9, -0.9),

    float2(-0.9, -0.9),
    float2(-0.1, -0.9),
    float2(-0.1, -0.1),

    float2(0.9, -0.1),
    float2(0.1, -0.1),
    float2(0.1, -0.9),

    float2(0.1, -0.9),
    float2(0.9, -0.9),
    float2(0.9, -0.1)
};

static const float2 uv[6] =
{
    float2(1.0f, 1.0f),
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),

    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f),
};

cbuffer ConstData
{
    float width;
    float height;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 uv : TEXCOORD0;
    uint texid : BLENDINDICES0;
};

VSOutput main(uint id : SV_VertexID)
{
    float xp = 2.0f / width;
    float yp = 2.0f / height;
    float disp = height / 3.0f;

    float kx = disp * xp;
    float ky = disp * yp;

    VSOutput output;
    output.position = float4(vertices[id % 24] * float2(kx, ky), 0, 1);
    output.uv = float4(uv[id % 6], 0, 0);
    output.texid = id / 6;
    return output;
}
