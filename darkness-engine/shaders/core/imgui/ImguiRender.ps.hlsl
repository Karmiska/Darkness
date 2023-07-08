
struct PSInput
{
    float4 pos   : SV_Position;
    float4 color : COLOR0;
    float2 uv    : TEXCOORD0;
};

Texture2D<float> tex;
sampler pointSampler;

float4 main(PSInput i) : SV_Target
{
    float  texColor = tex.Sample(pointSampler, i.uv);
    return texColor * i.color;
}
