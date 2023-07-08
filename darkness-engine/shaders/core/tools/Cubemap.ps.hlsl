
#include "../Common.hlsli"
#include "../renderers/Fog.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float4 pos              : NORMAL;
};

TextureCube<float4> cubemap;
TextureCube<float4> irradiance;
TextureCube<float4> convolution;
sampler cubemapSampler;

float4 main(PSInput input) : SV_Target
{
    float3 N = normalize(input.pos.xyz);
    N = float3(N.x, N.y, N.z);
    float3 cubeSample = cubemap.SampleLevel(cubemapSampler, N, 0).xyz;
    //float3 irradianceSample = irradiance.SampleLevel(cubemapSampler, N, 0).xyz;
    //float3 convolutionSample = convolution.SampleLevel(cubemapSampler, N, 0).xyz;

    float3 res = applyFog(cubeSample, 10000.0f, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
    //res += irradianceSample * 0.00001f;
    //res += convolutionSample * 0.00001f;

    return float4(res, 1.0f);
}
