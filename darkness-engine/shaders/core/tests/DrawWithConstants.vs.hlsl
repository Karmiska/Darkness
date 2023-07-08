
static const float4 vertices[3] =
{
    float4(.75f, -.75f, 0, 1),
    float4(0, .75f, 0, 1),
    float4(-.75f, -.75f, 0, 1),
};

cbuffer constants_struct
{
    float4 color1;
    float4 color2;
};

cbuffer constants_struct2
{
    float4 color3;
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
    o.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (id % 3 == 0)
    {
        o.color = color1;
    }
    else if (id % 3 == 1)
    {
        o.color = color2;
    }
    else if (id % 3 == 2)
    {
        o.color = color3;
    }
    return o;
}
