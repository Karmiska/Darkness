
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float4 pos              : NORMAL;
};

sampler environmentSampler;

TextureCube<float4> environmentMap;

float4 main(PSInput input) : SV_Target
{
    float3 N = normalize(input.pos.xyz);
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = cross(up, N);
    up = cross(N, right);

    float sampleDelta = 0.005;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < HALF_PI; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += environmentMap.Sample(environmentSampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    return float4(irradiance, 1.0f);
}
