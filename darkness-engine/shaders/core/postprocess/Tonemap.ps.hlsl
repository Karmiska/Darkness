
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float4> image;
sampler imageSampler;

float4 main(PSInput input) : SV_Target
{
    return float4(linearTosRGB(image.SampleLevel(imageSampler, input.uv, 0).xyz), 1.0f);
    //return float4(JimHejlRichardBurgessDawsonTonemap(image.SampleLevel(imageSampler, input.uv, 0).xyz), 1.0f);
    //return float4(ACESFitted(linearTosRGB(image.SampleLevel(imageSampler, input.uv, 0).xyz)), 1.0f);
}
