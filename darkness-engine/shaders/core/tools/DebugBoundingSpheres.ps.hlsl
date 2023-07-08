
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float4 mvPosCurrent     : POSITION0;
    float4 mvPosPrevious    : POSITION1;
    float4 normal           : NORMAL0;
    float4 tangent          : TEXCOORD0;
    float2 uv               : TEXCOORD1;
    uint clusterId          : BLENDINDICES0;
};

struct PsOutput
{
    float4 albedo;
    float4 normal;
    float2 motion;
    float roughness;
    float metalness;
    float occlusion;
};

float nrand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
};

[earlydepthstencil]
PsOutput main(PSInput input) : SV_Target
{
    // create tangent frame
    float3x3 TBN = float3x3(normalize(input.tangent.xyz), normalize(-cross(input.tangent.xyz, input.normal.xyz)), normalize(input.normal.xyz));
    TBN = transpose(TBN);

    PsOutput output;
    float randomValue1 = nrand(float2((float)input.clusterId + 0.1f, (float)input.clusterId + 0.4f));
    float randomValue2 = nrand(float2((float)input.clusterId + 0.2f, (float)input.clusterId + 0.5f));
    float randomValue3 = nrand(float2((float)input.clusterId + 0.3f, (float)input.clusterId + 0.6f));
    output.albedo = float4(randomValue1, randomValue2, randomValue3, 0.0f);
    output.normal = float4(normalize(input.normal.xyz), 1.0f);

    float2 currentPos = ((input.mvPosCurrent.xy / input.mvPosCurrent.w) * 0.5 + 0.5);
    currentPos = float2(currentPos.x, 1.0f - currentPos.y);
    float2 previousPos = ((input.mvPosPrevious.xy / input.mvPosPrevious.w) * 0.5 + 0.5);
    previousPos = float2(previousPos.x, 1.0f - previousPos.y);

    output.motion = 0.0f;// currentPos - previousPos;
    output.roughness = 1.0f;
    output.metalness = 0.0f;
    output.occlusion = 1.0f;
    return output;
}
