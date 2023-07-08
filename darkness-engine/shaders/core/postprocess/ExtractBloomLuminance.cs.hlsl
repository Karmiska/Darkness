
#include "../Common.hlsli"

sampler BiLinearClamp;
Texture2D<float4> source;
StructuredBuffer<float> Exposure;
RWTexture2D<float3> BloomResult;
RWTexture2D<uint> LumaResult;

cbuffer ExtractLuminanceConstants
{
    float2 inverseOutputSize;
    float bloomThreshold;
};

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // We need the scale factor and the size of one pixel so that our four samples are right in the middle
    // of the quadrant they are covering.
    float2 uv = (DTid.xy + 0.5) * inverseOutputSize;
    float2 offset = inverseOutputSize * 0.25f;

    // Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
    float3 color1 = source.SampleLevel(BiLinearClamp, uv + float2(-offset.x, -offset.y), 0).xyz;
    float3 color2 = source.SampleLevel(BiLinearClamp, uv + float2(offset.x, -offset.y), 0).xyz;
    float3 color3 = source.SampleLevel(BiLinearClamp, uv + float2(-offset.x, offset.y), 0).xyz;
    float3 color4 = source.SampleLevel(BiLinearClamp, uv + float2(offset.x, offset.y), 0).xyz;

    float luma1 = rgbToLuminance(color1);
    float luma2 = rgbToLuminance(color2);
    float luma3 = rgbToLuminance(color3);
    float luma4 = rgbToLuminance(color4);

    float ScaledThreshold = bloomThreshold * Exposure[1];    // BloomThreshold / Exposure

    // We perform a brightness filter pass, where lone bright pixels will contribute less.
    color1 *= max(FLT_EPS, luma1 - ScaledThreshold) / (luma1 + FLT_EPS);
    color2 *= max(FLT_EPS, luma2 - ScaledThreshold) / (luma2 + FLT_EPS);
    color3 *= max(FLT_EPS, luma3 - ScaledThreshold) / (luma3 + FLT_EPS);
    color4 *= max(FLT_EPS, luma4 - ScaledThreshold) / (luma4 + FLT_EPS);

    // The shimmer filter helps remove stray bright pixels from the bloom buffer by inversely weighting
    // them by their luminance.  The overall effect is to shrink bright pixel regions around the border.
    // Lone pixels are likely to dissolve completely.  This effect can be tuned by adjusting the shimmer
    // filter inverse strength.  The bigger it is, the less a pixel's luminance will matter.
    const float kShimmerFilterInverseStrength = 1.0f;
    float weight1 = 1.0f / (luma1 + kShimmerFilterInverseStrength);
    float weight2 = 1.0f / (luma2 + kShimmerFilterInverseStrength);
    float weight3 = 1.0f / (luma3 + kShimmerFilterInverseStrength);
    float weight4 = 1.0f / (luma4 + kShimmerFilterInverseStrength);
    float weightSum = weight1 + weight2 + weight3 + weight4;

    BloomResult[DTid.xy] = (color1 * weight1 + color2 * weight2 + color3 * weight3 + color4 * weight4) / weightSum;

    float luma = (luma1 + luma2 + luma3 + luma4) * 0.25;

    // Prevent log(0) and put only pure black pixels in Histogram[0]
    if (luma == 0.0)
    {
        LumaResult[DTid.xy] = 0;
    }
    else
    {
        const float MinLog = Exposure[4];
        const float RcpLogRange = Exposure[7];
        float logLuma = saturate((log2(luma) - MinLog) * RcpLogRange);    // Rescale to [0.0, 1.0]
        LumaResult[DTid.xy] = logLuma * 254.0 + 1.0;                    // Rescale to [1, 255]
    }
}
