
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

cbuffer Constants
{
    float2 onePerSize;
};

Texture2D<float4> image;
sampler imageSampler;

float4 main(PSInput input) : SV_Target
{
    float4 hdrColor01 = image.SampleLevel(imageSampler, input.uv, 0);
    float4 hdrColor02 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x, 0), 0);
    float4 hdrColor03 = image.SampleLevel(imageSampler, input.uv + float2(0, onePerSize.y), 0);
    float4 hdrColor04 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x, onePerSize.y), 0);

    float4 hdrColor11 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + 0, 0), 0);
    float4 hdrColor12 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + onePerSize.x, 0), 0);
    float4 hdrColor13 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + 0, onePerSize.y), 0);
    float4 hdrColor14 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + onePerSize.x, onePerSize.y), 0);

    float4 hdrColor21 = image.SampleLevel(imageSampler, input.uv + float2(0, onePerSize.y), 0);
    float4 hdrColor22 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x, onePerSize.y), 0);
    float4 hdrColor23 = image.SampleLevel(imageSampler, input.uv + float2(0, onePerSize.y + onePerSize.y), 0);
    float4 hdrColor24 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x, onePerSize.y + onePerSize.y), 0);

    float4 hdrColor31 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + 0, onePerSize.y), 0);
    float4 hdrColor32 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + onePerSize.x, onePerSize.y), 0);
    float4 hdrColor33 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + 0, onePerSize.y + onePerSize.y), 0);
    float4 hdrColor34 = image.SampleLevel(imageSampler, input.uv + float2(onePerSize.x + onePerSize.x, onePerSize.y + onePerSize.y), 0);

    float4 sum = 
        hdrColor01 + hdrColor02 + hdrColor03 + hdrColor04 +
        hdrColor11 + hdrColor12 + hdrColor13 + hdrColor14 +
        hdrColor21 + hdrColor22 + hdrColor23 + hdrColor24 +
        hdrColor31 + hdrColor32 + hdrColor33 + hdrColor34;
    float4 di = sum / 16;

    return di;
}
