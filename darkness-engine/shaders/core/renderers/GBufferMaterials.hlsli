
Texture2D<float2> gbufferNormals;
Texture2D<uint2> gbufferUV;
Texture2D<uint> gbufferInstanceId;

StructuredBuffer<InstanceMaterial> instanceMaterials;
Texture2D<float4> materialTextures[];

float rand_1_05(in float2 uv)
{
	float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	return abs(noise.x + noise.y) * 0.5;
}

#include "PartialDerivatives.hlsli"

void getMaterialsPartialDerivatives(
	// inputs
	uint instanceId,
	float2 uv,
	uint2 gUV,

	// outputs
	out float4 albedo,
	out float roughness,
	out float metalness,
	inout float ao
#ifdef OPTION_DEBUG
	, out float2 scaledUV
    , inout float albedoMip
    , inout float albedoMaxMip
    , inout float roughnessMip
    , inout float roughnessMaxMip
    , inout float metalnessMip
    , inout float metalnessMaxMip
    , inout float aoMip
    , inout float aoMaxMip
#endif
)
{
	InstanceMaterial material = instanceMaterials[instanceId];
	float2 materialScale = float2(material.scaleX, material.scaleY);

    float2 scaledUVLocal = uv * materialScale;

#ifdef OPTION_DEBUG
	scaledUV = scaledUVLocal;
#endif

	albedo = float4(material.color, 1.0f);
	roughness = material.roughnessStrength;
	metalness = material.metalnessStrength;

#ifdef OPTION_DEBUG
	if (debugMode == DEBUG_MODE_CLUSTERS)
	{
		albedo = float4(
			rand_1_05(float2(instanceId, instanceId)),
			rand_1_05(float2(instanceId + 1, instanceId)),
			rand_1_05(float2(instanceId + 2, instanceId)),
			1.0f);
	}
#endif

	float4 ddxDdy = calculateDdxDdyFromUV(uv, gUV, materialScale, instanceId);
	// log2(0.7) comes from ubi presentation about mip bias for temporal antialiasing
	// log2(0.7) == -0.51457317283

	uint width;
	uint height;

	const float mipBias = log2(0.7f);

	if ((material.materialSet & 0x1) == 0x1)
	{
#ifdef OPTION_DEBUG
        materialTextures[NonUniformResourceIndex(material.albedo)].GetDimensions(0, width, height, albedoMaxMip);
#else
        materialTextures[NonUniformResourceIndex(material.albedo)].GetDimensions(width, height);
#endif
		float texSize = (float)max(width, height);
        float mip = max(mipFromDdxDdy(ddxDdy * texSize) + mipBias, 0);

#ifdef OPTION_DEBUG
        albedoMip = mip;
#endif

		albedo *= materialTextures[NonUniformResourceIndex(material.albedo)].SampleLevel(tex_sampler, scaledUVLocal, mip);
	}

#ifdef OPTION_DEBUG
	if (debugMode == DEBUG_MODE_TRIANGLES)
	{
		albedo = float4(
			rand_1_05(float2(instanceId, instanceId + 1)),
			rand_1_05(float2(instanceId + 1, instanceId + 2)),
			rand_1_05(float2(instanceId + 2, instanceId + 3)),
			1.0f);
	}
#endif

	if ((material.materialSet & 0x4) == 0x4)
	{
#ifdef OPTION_DEBUG
        materialTextures[NonUniformResourceIndex(material.roughness)].GetDimensions(0, width, height, roughnessMaxMip);
#else
		materialTextures[NonUniformResourceIndex(material.roughness)].GetDimensions(width, height);
#endif
		float texSize = (float)max(width, height);
		float mip = max(mipFromDdxDdy(ddxDdy * texSize) + mipBias, 0);

#ifdef OPTION_DEBUG
        roughnessMip = mip;
#endif

		roughness = materialTextures[NonUniformResourceIndex(material.roughness)].SampleLevel(tex_sampler, scaledUVLocal, mip).r * material.roughnessStrength;
	}
	if ((material.materialSet & 0x2) == 0x2)
	{
#ifdef OPTION_DEBUG
        materialTextures[NonUniformResourceIndex(material.metalness)].GetDimensions(0, width, height, metalnessMaxMip);
#else
		materialTextures[NonUniformResourceIndex(material.metalness)].GetDimensions(width, height);
#endif
		float texSize = (float)max(width, height);
		float mip = max(mipFromDdxDdy(ddxDdy * texSize) + mipBias, 0);

#ifdef OPTION_DEBUG
        metalnessMip = mip;
#endif

		metalness = materialTextures[NonUniformResourceIndex(material.metalness)].SampleLevel(tex_sampler, scaledUVLocal, mip).r * material.metalnessStrength;
	}
	if ((material.materialSet & 0x10) == 0x10)
	{
#ifdef OPTION_DEBUG
        materialTextures[NonUniformResourceIndex(material.ao)].GetDimensions(0, width, height, aoMaxMip);
#else
		materialTextures[NonUniformResourceIndex(material.ao)].GetDimensions(width, height);
#endif
		float texSize = (float)max(width, height);
		float mip = max(mipFromDdxDdy(ddxDdy * texSize) + mipBias, 0);

#ifdef OPTION_DEBUG
        aoMip = mip;
#endif

		float ssaoSample = ao;
		float occlusionSample = materialTextures[NonUniformResourceIndex(material.ao)].SampleLevel(tex_sampler, scaledUVLocal, mip).r;

		// occlusion
		ao = 1.0f - ((1.0 - (occlusionSample - ssaoSample)) * material.occlusionStrength);
	}
	else
	{
		ao = 1.0f - ao;
	}

	roughness = min(max(roughness, 0.0f), 0.99);
	metalness = min(max(metalness, 0.0f), 1.0);
}
