
struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float4> texturea;
Texture2D<float4> textureb;
sampler samp;

float4 main(PSInput input) : SV_Target
{
    float3 color0 = texturea.SampleLevel(samp, input.uv, 0).xyz;
    float3 color1 = textureb.SampleLevel(samp, input.uv, 0).xyz;

    return float4(color0 + color1, 1.0f);
}
