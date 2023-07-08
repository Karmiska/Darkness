
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

cbuffer Constants
{
    float width;
    float height;
    uint horizontal;
};

Texture2D<float4> image;
sampler imageSampler;

static const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

float4 main(PSInput input) : SV_Target
{
    float2 offset = 1.0 / float2(width, height);
    float3 result = image.Sample(imageSampler, input.uv).xyz * weight[0];
    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += image.Sample(imageSampler, input.uv + float2(offset.x * i, 0.0)).xyz * weight[i];
            result += image.Sample(imageSampler, input.uv - float2(offset.x * i, 0.0)).xyz * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += image.Sample(imageSampler, input.uv + float2(0.0, offset.y * i)).xyz * weight[i];
            result += image.Sample(imageSampler, input.uv - float2(0.0, offset.y * i)).xyz * weight[i];
        }
    }
    return float4(result, 1.0);
}
