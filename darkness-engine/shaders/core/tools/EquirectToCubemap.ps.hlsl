#include "../Common.hlsli"

struct PSInput
{
    float4 position : SV_Position0;
    float4 normal   : NORMAL;
};

sampler equirectangularSampler;

Texture2D<float4> equirectangularMap;

float4 main(PSInput input) : SV_Target
{
    float3 inputNormal = normalize(input.normal.xyz);
    float3 N = float3(inputNormal.x, inputNormal.y, inputNormal.z);
    float2 uv = envMapEquirect(N);
    float3 color = equirectangularMap.Sample(equirectangularSampler, uv).rgb;
    return float4(color, 1.0f);
}
