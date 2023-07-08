
struct PSInput
{
	float4 position         : SV_Position0;
	float2 uv               : TEXCOORD0;
};

sampler tex_sampler;

Texture2D<uint2> gbufferUV;
Texture2D<uint> gbufferInstanceId;

#include "../shared_types/InstanceMaterial.hlsli"

StructuredBuffer<InstanceMaterial> instanceMaterials;
Texture2D<float4> materialTextures[];

#include "../renderers/PartialDerivatives.hlsli"

float2 main(PSInput input) : SV_Target
{
	uint2 gUV = uint2((uint)input.position.x, (uint)input.position.y);
	uint instanceId = gbufferInstanceId[gUV * 2];
	float2 objectUV = gbufferUV[gUV * 2] / 65535.0f;

	InstanceMaterial material = instanceMaterials[instanceId];
	float2 materialScale = float2(material.scaleX, material.scaleY);
	float2 scaledUV = objectUV * materialScale;

	float roughness = material.roughnessStrength;
	float metalness = material.metalnessStrength;
	float4 ddxDdy = calculateDdxDdyFromUV(objectUV, gUV * 2, materialScale, instanceId);
	uint width;
	uint height;
	if ((material.materialSet & 0x4) == 0x4)
	{
		materialTextures[NonUniformResourceIndex(material.roughness)].GetDimensions(width, height);
		float texSize = (float)max(width, height);
		float mip = max(mipFromDdxDdy(ddxDdy * texSize) + log2(0.70), 0);
		roughness = materialTextures[NonUniformResourceIndex(material.roughness)].SampleLevel(tex_sampler, scaledUV, mip).r * material.roughnessStrength;
	}
	if ((material.materialSet & 0x2) == 0x2)
	{
		materialTextures[NonUniformResourceIndex(material.metalness)].GetDimensions(width, height);
		float texSize = (float)max(width, height);
		float mip = max(mipFromDdxDdy(ddxDdy * texSize) + log2(0.70), 0);
		metalness = materialTextures[NonUniformResourceIndex(material.metalness)].SampleLevel(tex_sampler, scaledUV, mip).r * material.metalnessStrength;
	}

	roughness = min(max(roughness, 0.0f), 0.99);
	metalness = min(max(metalness, 0.0f), 1.0);

	return float2(roughness, metalness);
}
