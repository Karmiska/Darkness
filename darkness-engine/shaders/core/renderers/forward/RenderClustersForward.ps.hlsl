
#include "../../Common.hlsli"
#include "../../shared_types/ClusterInstanceData.hlsli"
#include "../../shared_types/ClusterData.hlsli"
#include "../../shared_types/InstanceMaterial.hlsli"
#include "../../shared_types/DebugModes.hlsli"
#include "../../PhysicallyBasedRendering.hlsli"

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
	float4 color;
	float2 motion;
};

TextureCube<float4> environmentIrradianceCubemap;
Texture2D<float4> environmentIrradiance;
TextureCube<float4> environmentSpecular;
Texture2D<float4> environmentBrdfLut;

Texture2D<float> ssao;

#include "../Lighting.hlsli"

cbuffer ConstData
{
	float3 cameraWorldSpacePosition;
	float exposure;

	uint hasEnvironmentIrradianceCubemap;
	uint hasEnvironmentIrradianceEquirect;
	uint hasEnvironmentSpecular;
	float environmentStrength;

	float4 probePositionRange;
	float4 probeBBmin;
	float4 probeBBmax;

	float4x4 cameraInverseProjectionMatrix;
	float4x4 cameraInverseViewMatrix;

	uint2 frameSize;
	uint usingProbe;
	uint debugMode;

	float3 resolution;
	float padding;
};

sampler tex_sampler;
sampler tri_sampler;
sampler point_sampler;

float3 sampleEnvironment(float3 direction, float lod)
{
	float3 inverseDirection = float3(direction.x, direction.y, -direction.z);
	if (hasEnvironmentIrradianceCubemap)
		return environmentIrradianceCubemap.SampleLevel(tri_sampler, inverseDirection, lod).xyz;
	else if (hasEnvironmentIrradianceEquirect)
		return environmentIrradiance.SampleLevel(tri_sampler, envMapEquirect(direction), lod).xyz;
	else
		return float3(0.0f, 0.0f, 0.0f);
}

#include "../../GBuffer.hlsli"
#include "../Normalmapping.hlsli"
#include "../Materials.hlsli"
#include "../Fog.hlsli"

[earlydepthstencil]
PSOutput main(PSInput input, bool isFrontFace : SV_IsFrontFace) : SV_Target
{
#ifdef OPTION_DEBUG
#endif

	// UV
	float2 objectUV = float2(input.normal.w, input.tangent.w);
	float2 screenUV = input.position.xy / resolution.xy;

	// TODO: trying not to draw too far objects
	//		 should be unnecessary
	float depthSample = input.position.z;
	clip(depthSample - 0.0001);

	// get screen space ambient occlusion
	float occlusion = ssao.Sample(tex_sampler, screenUV);

	float3 normal = normalize(input.normal.xyz);
	float3 tangent = normalize(input.tangent.xyz);

	// get object material values
	float3 normalSample = normal;
	float4 albedo;
	float roughness;
	float metalness;
#ifdef OPTION_DEBUG
	float2 scaledObjectUV;

    float albedoMip = 0.0f;
    float albedoMaxMip = 0.0f;

    float roughnessMip = 0.0f;
    float roughnessMaxMip = 0.0f;

    float metalnessMip = 0.0f;
    float metalnessMaxMip = 0.0f;

    float aoMip = 0.0f;
    float aoMaxMip = 0.0f;
#endif

	bool hasNormal = getMaterials(
#ifdef OPTION_DEBUG
        (debugMode == DEBUG_MODE_CLUSTERS) ? input.clusterPointer : input.instancePointer,
#else
        input.instancePointer,
#endif
		objectUV,
		normalSample,
		albedo, 
		roughness, 
		metalness, 
		occlusion
#ifdef OPTION_DEBUG
		, scaledObjectUV
#endif
	);

	// create normal from normal map sample and object normal/tangent
	if (hasNormal)
		normal = normalMap(
			isFrontFace,
			objectUV.x < 0.0,
			normalSample,
			normal,
			tangent);

	// calculate world position
	float2 inverseSize = float2(1.0f / (float)frameSize.x, 1.0f / (float)frameSize.y);
	float3 inputpos = float3(
		(input.position.x * inverseSize.x) * 2.0f - 1.0f,
		((input.position.y * inverseSize.y) * 2.0f - 1.0f) * -1.0f,
		depthSample);
	float4 ci = mul(cameraInverseProjectionMatrix, float4(inputpos.xyz, 1.0f));
	ci.xyz /= ci.w;
	float3 worldPosition = mul(cameraInverseViewMatrix, float4(ci.xyz, 0.0f)).xyz + cameraWorldSpacePosition;

	// setup lighting values
	float3 V = normalize(cameraWorldSpacePosition - worldPosition);
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo.xyz, metalness);
	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	uint drawnLights = performLighting(
		worldPosition, 
		cameraWorldSpacePosition, 
		normal, V, 
		albedo.xyz,
		metalness,
		roughness,
		uint2((uint)input.position.x, (uint)input.position.y), 
		F0, Lo);
	

	// ambient specular
	const float MaxReflectionLod = 4;
	float3 from = worldPosition;
	float3 probePosition = probePositionRange.xyz;
	float probeRange = probePositionRange.w;

	// cubemap parallax correction
	float3 Rp = reflect(-V, normal);
	float3 newRp;
	float3 probeValue;
	if (usingProbe)
	{
		newRp = boxLineIntersect(probeBBmin.xyz, probeBBmax.xyz, worldPosition, -Rp, probePosition);
		probeValue = float3(-newRp.x, -newRp.y, -newRp.z);
	}
	else
	{
		newRp = boxLineIntersect(probeBBmin.xyz, probeBBmax.xyz, worldPosition, Rp, probePosition);
		probeValue = float3(newRp.x, newRp.y, -newRp.z);
	}


	float3 ssrSample = float3(0.0f, 0.0f, 0.0f);

	#include "../EnvironmentIBL.hlsli"

	color = applyFog(color, length(cameraWorldSpacePosition - worldPosition), cameraWorldSpacePosition, V);

	PSOutput output;

	float2 cur = (input.mvPosCurrent.xy / input.mvPosCurrent.w) * 0.5 + 0.5;
	float2 pre = (input.mvPosPrevious.xy / input.mvPosPrevious.w) * 0.5 + 0.5;
	cur.y = 1.0f - cur.y;
	pre.y = 1.0f - pre.y;
	output.motion = cur - pre;

	float4 outputColor;
	#include "../ColorOutput.hlsli"
	output.color = outputColor;

	return output;
}
