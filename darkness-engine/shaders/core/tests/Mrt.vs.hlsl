
static const float2 vertices[4] =
{
    float2(-0.5,  0.5),
    float2(0.5,  0.5),
    float2(-0.5, -0.5),
    float2(0.5, -0.5)
};

static const float2 uv[4] =
{
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 1.0f),
};

struct VSOutput
{
    float4 position : SV_Position;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.position = float4(vertices[id % 4], 0.0f, 1.0f);
    return output;
}
