
struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float4> src;
sampler samp;

float4 main(PSInput input) : SV_Target
{
    return src.SampleLevel(samp, input.uv, 0);
}
