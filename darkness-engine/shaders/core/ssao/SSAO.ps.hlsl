
#include "../Common.hlsli"
#include "../GBuffer.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float4 viewRay          : TEXCOORD0;
    float2 uv               : TEXCOORD1;
    
};

Texture2D<float> depthTexture;
Texture2D<float2> normalTexture;
Texture2D<float4> noiseTexture;
Buffer<float4> samples;
sampler ssaoSampler;
sampler depthSampler;
sampler defaultSampler;

cbuffer Constants
{
    float4x4 cameraProjectionMatrix;
    float4x4 cameraViewMatrix;
    float2 frameSize;
    float2 nearFar;
};

float linDepth(float depth, float2 nearFar)
{
    return 1.0 / (((nearFar.y - nearFar.x) - nearFar.x) * depth + 1.0);
}

float3 getNormal(float2 uv)
{
    return unpackNormalOctahedron(normalTexture.Sample(defaultSampler, uv));
}

float main(PSInput input) : SV_Target
{
    float occlusion = 0.0f;
    int kernelSize = 8;
    float radius = 0.010;
    float bias = 0.00025;

    float2 uv = input.uv;
    float3 viewRay = float4(normalize(input.viewRay.xyz), 0).xyz;
    float3 position = viewRay * linDepth(depthTexture.Sample(depthSampler, uv).x, nearFar);
    
    float3 normal = getNormal(uv);

    float2 noiseScale = frameSize / 4.0;
    float3 randomVec = noiseTexture.Sample(ssaoSampler, uv * noiseScale).xyz;

    normal = mul(cameraViewMatrix, float4(normal, 0)).xyz;
    float3 ranVec = randomVec - (normal * dot(randomVec, normal));
    float3 tangent = normalize(ranVec);
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    TBN = transpose(TBN);

    float3 de;

    for (int i = 0; i < kernelSize; ++i)
    {
        float3 samp = mul(TBN, samples[i].xyz);
        samp = position + (normal * bias) + (samp * radius);

        float4 offset = float4(samp, 1.0);
        offset = mul(cameraProjectionMatrix, offset);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        offset.y = 1.0f - offset.y;

        float3 depthSample = viewRay * linDepth(depthTexture.Sample(depthSampler, offset.xy).x, nearFar);

        float rangeCheck = abs(depthSample.z - samp.z) < radius ? 1.0 : 0.0;
        occlusion += (samp.z < depthSample.z ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = (occlusion / kernelSize) * 1.3f;
    
    return occlusion;
}
