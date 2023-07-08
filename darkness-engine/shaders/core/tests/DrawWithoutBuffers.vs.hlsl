
static const float4 vertices[3] =
{
    float4(.75f, -.75f, 0, 1),
    float4(0, .75f, 0, 1),
    float4(-.75f, -.75f, 0, 1),
};

static const float4 colors[3] =
{
    float4(1.0f, 0.0f, 0.0f, 1.0f),
    float4(0.0f, 1.0f, 0.0f, 1.0f),
    float4(0.0f, 0.0f, 1.0f, 1.0f),
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput o;
    o.position = vertices[id % 3];
    o.color = colors[id % 3];
    return o;
}
