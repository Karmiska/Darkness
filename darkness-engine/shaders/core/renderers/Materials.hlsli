
StructuredBuffer<InstanceMaterial> instanceMaterials;
Texture2D<float4> materialTextures[];

float rand_1_05(in float2 uv)
{
	float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	return abs(noise.x + noise.y) * 0.5;
}

bool getMaterials(
	// inputs
	uint instanceId, 
	float2 uv, 

	// outputs
	inout float3 normal, 
	out float4 albedo, 
	out float roughness, 
	out float metalness, 
	inout float ao
#ifdef OPTION_DEBUG
	, out float2 scaledUV
#endif
)
{
	bool res = false;
	InstanceMaterial material = instanceMaterials[instanceId];
	float2 materialScale = float2(material.scaleX, material.scaleY);

#ifdef OPTION_DEBUG
	scaledUV = uv * materialScale;
#else
	float2 scaledUV = uv * materialScale;
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

	if ((material.materialSet & 0x8) == 0x8)
	{
		normal = materialTextures[NonUniformResourceIndex(material.normal)].Sample(tex_sampler, scaledUV).xyz;
		res = true;
	}
	if ((material.materialSet & 0x1) == 0x1)
	{
		albedo *= materialTextures[NonUniformResourceIndex(material.albedo)].Sample(tex_sampler, scaledUV);
	}
	if ((material.materialSet & 0x4) == 0x4)
	{
		roughness = materialTextures[NonUniformResourceIndex(material.roughness)].Sample(tex_sampler, scaledUV).r * material.roughnessStrength;
	}
	if ((material.materialSet & 0x2) == 0x2)
	{
		metalness = materialTextures[NonUniformResourceIndex(material.metalness)].Sample(tex_sampler, scaledUV).r * material.metalnessStrength;
	}
	if ((material.materialSet & 0x10) == 0x10)
	{
		float ssaoSample = ao;
		float occlusionSample = materialTextures[NonUniformResourceIndex(material.ao)].Sample(tex_sampler, scaledUV).r;

		// occlusion
		ao = 1.0f - ((1.0 - (occlusionSample - ssaoSample)) * material.occlusionStrength);
	}
	else
	{
		ao = 1.0f - ao;
	}

	roughness = min(max(roughness, 0.0f), 0.99);
	metalness = min(max(metalness, 0.0f), 1.0);

	return res;
}
