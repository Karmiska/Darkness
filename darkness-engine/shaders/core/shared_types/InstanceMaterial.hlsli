#ifdef __cplusplus
#pragma once
#endif

struct InstanceMaterial
{
    uint albedo;
	uint albedoAlphaMask;
    uint metalness;
    uint roughness;

    uint normal;
    uint ao;
    uint materialSet;
    float roughnessStrength;

    float metalnessStrength;
    float occlusionStrength;
    float scaleX;
    float scaleY;

	float3 color;
	float padding;
};
