
#include "../Common.hlsli"

sampler BiLinearClamp;
Texture2D<float3> source;
StructuredBuffer<float> Exposure;
RWTexture2D<uint> LumaResult;

cbuffer ExtractLuminanceConstants
{
    float2 inverseOutputSize;
};

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // We need the scale factor and the size of one pixel so that our four samples are right in the middle
    // of the quadrant they are covering.
    float2 uv = DTid.xy * inverseOutputSize;
    float2 offset = inverseOutputSize * 0.25f;

    // Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
    float3 color1 = source.SampleLevel(BiLinearClamp, uv + float2(-offset.x, -offset.y), 0);
    float3 color2 = source.SampleLevel(BiLinearClamp, uv + float2(offset.x, -offset.y), 0);
    float3 color3 = source.SampleLevel(BiLinearClamp, uv + float2(-offset.x, offset.y), 0);
    float3 color4 = source.SampleLevel(BiLinearClamp, uv + float2(offset.x, offset.y), 0);

    // Compute average luminance
    float luma = rgbToLuminance(color1 + color2 + color3 + color4) * 0.25;

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
