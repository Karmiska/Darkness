
cbuffer constants_struct
{
    float2 reciprocalResolution;
};

struct VSInput
{
    float2 pos   : POSITION0;
    float2 uv    : TEXCOORD0;
    uint color : COLOR0;
};

struct VSOutput
{
    float4 pos   : SV_Position;
    float4 color : COLOR0;
    float2 uv    : TEXCOORD0;
    
};

VSOutput main(VSInput i)
{
    VSOutput o;

    float2 fracPos = i.pos * reciprocalResolution;
    float2 clipPos = lerp(float2(-1, 1), float2(1, -1), fracPos);

    o.uv = i.uv.xy;

    o.color = float4(
        ((float)(i.color & 0x000000ff) / 255.0f),
        ((float)((i.color & 0x0000ff00) >> 8) / 255.0f),
        ((float)((i.color & 0x00ff0000) >> 16) / 255.0f),
        ((float)((i.color & 0xff000000) >> 24) / 255.0f));

    o.pos = float4(clipPos, 0, 1);

    return o;
}
