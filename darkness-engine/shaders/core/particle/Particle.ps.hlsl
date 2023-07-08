
#include "../Common.hlsli"

struct PSInput
{
    float4 position : SV_Position0;
    float4 normal   : NORMAL0;
    float4 tangent  : NORMAL1;
    float2 uv       : TEXCOORD0;
};

cbuffer ConstData
{
    float4x4 cameraInverseProjectionMatrix;
    float4x4 cameraInverseViewMatrix;
    float4 cameraPosition;
    float2 inverseSize;
    uint animationIndex;
    uint lightCount;
    float2 shadowSize;
    uint hasEnvironmentIrradianceCubemap;
    uint hasEnvironmentIrradianceEquirect;
    uint hasEnvironmentSpecular;
    float environmentStrength;
};

TextureCube<float4> environmentIrradianceCubemap;
Texture2D<float4> environmentIrradiance;
TextureCube<float4> environmentSpecular;
Texture2D<float4> environmentBrdfLut;

Buffer<float4> lightWorldPosition;
Buffer<float4> lightDirection;
Buffer<float4> lightColor;
Buffer<float4> lightParameters;
Buffer<uint> lightType;
Buffer<float> lightIntensity;
Buffer<float> lightRange;

Texture2DArray<float4> shadowMap;
StructuredBuffer<float4x4> shadowVP;

Texture2D<float> depth;
Texture2D<float4> diffuse[40];
Texture2D<float4> normal[40];
Texture2D<float4> alpha[40];

Texture2D<float4> top[40];
Texture2D<float4> left[40];
Texture2D<float4> bottom[40];
Texture2D<float4> right[40];

sampler particleSampler;
sampler triSampler;
SamplerComparisonState shadowSampler;

float3 sampleEnvironment(float3 direction, float lod)
{
    float3 inverseDirection = float3(direction.x, direction.y, -direction.z);
    if (hasEnvironmentIrradianceCubemap)
        return environmentIrradianceCubemap.SampleLevel(triSampler, inverseDirection, lod).xyz;
    else if (hasEnvironmentIrradianceEquirect)
        return environmentIrradiance.SampleLevel(triSampler, envMapEquirect(direction), lod).xyz;
    else
        return float3(0.0f, 0.0f, 0.0f);
}

float4 main(PSInput input) : SV_Target
{
    float3 inputpos = float3(
        (input.position.x * inverseSize.x) * 2.0f - 1.0f,
        ((input.position.y * inverseSize.y) * 2.0f - 1.0f) * -1.0f,
        input.position.z);

    float depthParticle = inputpos.z;

    float2 posFrag = inputpos.xy * float2(0.5, -0.5) + float2(0.5, 0.5);
    float depthScene = depth.Sample(particleSampler, posFrag);

    float2 uv = float2(input.uv.x, 1.0f - input.uv.y);
    float3 diffuseSample = diffuse[animationIndex].Sample(particleSampler, uv).xyz;
    float3 normalSample = normal[animationIndex].Sample(particleSampler, uv).xyz;
    float3 alphaSample = alpha[animationIndex].Sample(particleSampler, uv).xyz;


    // lighting
    float4 ci = mul(cameraInverseProjectionMatrix, float4(inputpos.xy, depthParticle, 1.0f));
    ci.xyz /= ci.w;
    float3 worldPosition = mul(cameraInverseViewMatrix, float4(ci.xyz, 0.0f)).xyz + cameraPosition.xyz;


    float3 N = input.normal.xyz;
    float3 V = normalize(cameraPosition.xyz - worldPosition);

    float3x3 TBN = float3x3(
        normalize(input.tangent.xyz),
        normalize(cross(input.tangent.xyz, input.normal.xyz)),
        normalize(input.normal.xyz)
        );
    TBN = transpose(TBN);
    N = mul(TBN, normalize(normalSample)).xyz;
    N = float3(N.x, -N.y, N.z);

    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    uint shadowCasterIndex = 0;
    for (uint i = 0; i < lightCount; ++i)
    {
        float distance = length(lightWorldPosition[i].xyz - worldPosition);
        float distanceSqrd = distance * distance;
        float attenuation = 1.0 / distanceSqrd;

        float rangeSqrd = lightRange[i] * lightRange[i];
        float overDistance = distanceSqrd - rangeSqrd;
        const float dimArea = lightRange[i] * 0.1;
        float inDim = 1.0f - saturate((dimArea - overDistance) / dimArea);
        attenuation *= 1.0f - inDim;

        if (lightType[i] == 0)
        {
            // directional light
            float3 L = normalize(lightDirection[i].xyz);
            float3 H = normalize(V + L);
            float3 radiance = (lightColor[i].xyz * lightIntensity[i]);

            float NdotL = max(dot(N, L), 0.0);
            float3 directContrib = diffuseSample * radiance * NdotL;
            Lo += directContrib;
        }
        else if (lightType[i] == 1)
        {
            // spot light
            float3 L = normalize(lightWorldPosition[i].xyz - worldPosition);
            float3 H = normalize(V + L);

            float3 radiance = (lightColor[i].xyz * lightIntensity[i]) * attenuation;

            float3 spotDirection = normalize(lightDirection[i].xyz);
            float cos_cur_angle = dot(L, spotDirection);

            float outer = lightParameters[i].y;
            float inner = lightParameters[i].x;
            bool shadowCasting = lightParameters[i].z > 0.0f;

            float angleDegrees = degrees(acos(cos_cur_angle));
            float angleDistance = outer - inner;

            if (angleDegrees < lightParameters[i].y)
            {
                float spot = clamp((outer - angleDegrees) / angleDistance, 0.0f, 1.0f);

                float NdotL = max(dot(N, L), 0.0);
                float3 directContrib = diffuseSample * radiance * NdotL * spot;

                // shadow bias
                float hi = 0.00005;
                float lo = 0.000005;
                float bias = lerp(hi, lo, pow(NdotL, 0.2));

                // spot shadow
                if (shadowCasting)
                {
                    //directContrib *= 1.0 - ((shadowSample | (1 << i)) >> i);
                    float4 shadowPosition = mul(float4(worldPosition, 1.0), shadowVP[shadowCasterIndex]);
                    float2 shadowUV = (0.5 * (shadowPosition.xy / shadowPosition.w)) + float2(0.5, 0.5);

                    shadowUV.y = 1.0f - shadowUV.y;
                    float2 satShadowUV = saturate(shadowUV);

                    if (satShadowUV.x == shadowUV.x && satShadowUV.y == shadowUV.y && (shadowPosition.z / shadowPosition.w > 0))
                    {
                        //PCF sampling for shadow map
                        float sum = 0;
                        float z = (shadowPosition.z / shadowPosition.w) + bias;
                        float x, y;

                        for (y = -1.5f; y <= 1.5f; y += 1.0f)
                        {
                            for (x = -1.5f; x <= 1.5f; x += 1.0f)
                            {
                                sum += shadowMap.SampleCmpLevelZero(
                                    shadowSampler,
                                    float3(shadowUV + (float2(x, y)*shadowSize), shadowCasterIndex),
                                    z);
                            }
                        }
                        float shadowFactor = sum / 16.0f;

                        directContrib *= 1.0f - shadowFactor;
                    }
                }

                Lo += directContrib;
            }
            if (shadowCasting)
                shadowCasterIndex += 1;
        }
        else if (lightType[i] == 2)
        {
            // point light
            float3 L = normalize(lightWorldPosition[i].xyz - worldPosition);
            float3 H = normalize(V + L);

            float3 radiance = (lightColor[i].xyz * lightIntensity[i]) * attenuation;

            float NdotL = max(dot(N, L), 0.0);
            float3 directContrib = diffuseSample * radiance * NdotL;

            // point shadows
            bool shadowCasting = lightParameters[i].z > 0.0f;
            if (shadowCasting)
            {
                // shadow bias
                float hi = 0.00030;
                float lo = 0.00025;
                float bias = lerp(hi, lo, pow(NdotL, 0.2));

                for (int caster = 0; caster < 6; ++caster)
                {
                    float4 shadowPosition = mul(float4(worldPosition, 1.0), shadowVP[shadowCasterIndex]);
                    float2 shadowUV = (0.5 * (shadowPosition.xy / shadowPosition.w)) + float2(0.5, 0.5);

                    shadowUV.y = 1.0f - shadowUV.y;
                    float2 satShadowUV = saturate(shadowUV);

                    if (satShadowUV.x == shadowUV.x && satShadowUV.y == shadowUV.y && (shadowPosition.z / shadowPosition.w > 0))
                    {
                        //return float4(1.0f, 0.0f, 0.0f, 1.0f);

                        //PCF sampling for shadow map
                        float sum = 0;
                        float z = (shadowPosition.z / shadowPosition.w) + bias;
                        float x, y;

                        for (y = -1.5f; y <= 1.5f; y += 1.0f)
                        {
                            for (x = -1.5f; x <= 1.5f; x += 1.0f)
                            {
                                sum += shadowMap.SampleCmpLevelZero(
                                    shadowSampler,
                                    float3(shadowUV + (float2(x, y)*shadowSize), shadowCasterIndex),
                                    z);
                            }
                        }
                        float shadowFactor = sum / 16.0f;

                        directContrib *= 1.0f - shadowFactor;
                    }
                    shadowCasterIndex++;
                }
            }

            Lo += directContrib;
        }
    }

    // ambient lighting
    float3 environIrradianceSample = sampleEnvironment(N, 0.0f);
    float3 diffuse = environIrradianceSample * diffuseSample;

    float3 ambient = max(diffuse * environmentStrength, float3(0.0, 0.0, 0.0));
    //float3 ambient = diffuse;

    float3 color = (ambient + Lo);

    // output
    float fade = saturate(1000 * saturate(depthParticle - depthScene));
    return float4(color, alphaSample.x * fade);
    //return float4(Lo, 1.0f);
}
