
static const float4 vertices[3] =
{
    float4(.75f, -.75f, 0, 1),
    float4(0, .75f, 0, 1),
    float4(-.75f, -.75f, 0, 1),
};

struct VSOutput
{
    float4 position : SV_Position;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput o;
    o.position = vertices[id % 3];
    return o;
}
