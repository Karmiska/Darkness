
#include "../../Common.hlsli"
#include "../../shared_types/ClusterInstanceData.hlsli"
#include "../../shared_types/ClusterData.hlsli"
#include "../../shared_types/InstanceMaterial.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float4 mvPosCurrent     : POSITION0;
    float4 mvPosPrevious    : POSITION1;
    float4 normal           : NORMAL0;
    float4 tangent          : TEXCOORD0;
    uint instancePointer    : BLENDINDICES0;
#ifdef OPTION_DEBUG
    uint clusterPointer     : BLENDINDICES1;
#endif
};

struct PSOutput
{
    float2 normal;
    uint2 uv;
    float2 motion;
    uint instanceId;
};

StructuredBuffer<InstanceMaterial> instanceMaterials;
Texture2D<float4> materialTextures[];

sampler tex_sampler;

float nrand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
};

#include "../../GBuffer.hlsli"

PSOutput main(PSInput input, bool isFrontFace : SV_IsFrontFace) : SV_Target
{
	float2 inuv = float2(input.normal.w, input.tangent.w);

    InstanceMaterial material = instanceMaterials[input.instancePointer];
    float2 materialScale = float2(material.scaleX, material.scaleY);

    PSOutput output;

    float3 normal = input.normal.xyz;
    float3 tangent = input.tangent.xyz;
    if (!isFrontFace)
        normal *= -1.0f;

	float mirrorUV = -1.0f;
	if (inuv.x < 0.0)
		mirrorUV = 1.0f;

    if ((material.materialSet & 0x8) == 0x8)
    {
        // create tangent frame
        float3x3 TBN = float3x3(
            normalize(tangent),
            normalize(cross(tangent, normal) * mirrorUV),
            normalize(normal));
        TBN = transpose(TBN);

        // create normal from normal map sample
        float4 normalSample = materialTextures[NonUniformResourceIndex(material.normal)].Sample(tex_sampler, inuv * materialScale);
        normal = createNormal(TBN, normalSample.xyz);
    }
    normal = abs(normal);

    output.normal = packNormalOctahedron(normalize(normal));
    output.uv = uint2(65535.0f * frac(inuv.x), 65535.0f * frac(inuv.y));

    float2 cur = (input.mvPosCurrent.xy / input.mvPosCurrent.w) * 0.5 + 0.5;
    float2 pre = (input.mvPosPrevious.xy / input.mvPosPrevious.w) * 0.5 + 0.5;
    cur.y = 1.0f - cur.y;
    pre.y = 1.0f - pre.y;
    output.motion = cur - pre;
    
	output.instanceId = input.instancePointer;

#ifdef OPTION_DEBUG
    output.instanceId = input.clusterPointer;
#endif

    return output;
}
