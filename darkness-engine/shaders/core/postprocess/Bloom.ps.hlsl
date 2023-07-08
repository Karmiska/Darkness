
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float4> framebuffer;
Texture2D<float4> blur;
StructuredBuffer<float> Exposure;
sampler framebufferSampler;

float4 main(PSInput input) : SV_Target
{
    float3 hdrColor = framebuffer.SampleLevel(framebufferSampler, input.uv, 0).xyz;
    float3 blurColor0 = blur.Sample(framebufferSampler, input.uv, 0).xyz;
    //float3 blurColor1 = blur.SampleLevel(framebufferSampler, input.uv, 1).xyz;
    //float3 blurColor2 = blur.SampleLevel(framebufferSampler, input.uv, 2).xyz;
    //float3 blurColor3 = blur.SampleLevel(framebufferSampler, input.uv, 3).xyz;
    //float3 blurColor4 = blur.SampleLevel(framebufferSampler, input.uv, 4).xyz;

    //return float4(linearTosRGB(hdrColor + blurColor), 1.0f);
    float3 outputColor = hdrColor + blurColor0;
    outputColor *= Exposure[0];

    return float4(JimHejlRichardBurgessDawsonTonemap(outputColor), 1.0f);
    //return float4(ACESFitted(linearTosRGB(hdrColor + blurColor)), 1.0f);
}
