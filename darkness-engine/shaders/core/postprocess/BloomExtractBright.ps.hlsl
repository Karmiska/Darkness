
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

cbuffer BloomExtractConstants
{
    float2 invSize;
    float bloomThreshold;
    float exposure;
};

Texture2D<float4> framebuffer;
sampler framebufferSampler;

float4 main(PSInput input) : SV_Target
{
    float2 uv = input.uv;

    float3 color0 = framebuffer.SampleLevel(framebufferSampler, uv + float2(0.0f, 0.0f), 0).xyz;
    float3 color1 = framebuffer.SampleLevel(framebufferSampler, uv + float2(invSize.x, 0.0f), 0).xyz;
    float3 color2 = framebuffer.SampleLevel(framebufferSampler, uv + float2(0.0f, invSize.y), 0).xyz;
    float3 color3 = framebuffer.SampleLevel(framebufferSampler, uv + float2(invSize.x, invSize.y), 0).xyz;

    float luma0 = rgbToLuminance(color0);
    float luma1 = rgbToLuminance(color1);
    float luma2 = rgbToLuminance(color2);
    float luma3 = rgbToLuminance(color3);

    float scaledThreshold = bloomThreshold * exposure;

    // brightness filter
    color0 *= max(FLT_EPS, luma0 - scaledThreshold) / (luma0 + FLT_EPS);
    color1 *= max(FLT_EPS, luma1 - scaledThreshold) / (luma1 + FLT_EPS);
    color2 *= max(FLT_EPS, luma2 - scaledThreshold) / (luma2 + FLT_EPS);
    color3 *= max(FLT_EPS, luma3 - scaledThreshold) / (luma3 + FLT_EPS);

    // shimmer filter
    const float shimmerFilterInverseStrength = 1.0f;
    float weight0 = 1.0f / (luma0 + shimmerFilterInverseStrength);
    float weight1 = 1.0f / (luma1 + shimmerFilterInverseStrength);
    float weight2 = 1.0f / (luma2 + shimmerFilterInverseStrength);
    float weight3 = 1.0f / (luma3 + shimmerFilterInverseStrength);
    float weightSum = weight0 + weight1 + weight2 + weight3;

    float3 result = (
        color0 * weight0 +
        color1 * weight1 +
        color2 * weight2 +
        color3 * weight3) / weightSum;

    float luma = (luma0 + luma1 + luma2 + luma3) * 0.25;

    if(luma > bloomThreshold)
        return float4(result, 1.0f);
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}
