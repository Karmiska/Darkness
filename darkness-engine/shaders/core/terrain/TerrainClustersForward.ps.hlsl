
#include "../Common.hlsli"
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/ClusterData.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../shared_types/DebugModes.hlsli"
#include "../PhysicallyBasedRendering.hlsli"
#include "../shared_types/TransformHistory.hlsli"

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
StructuredBuffer<TransformHistory> transformHistory;

Texture2D<float4> colormap;
Texture2D<float> heightmap;

#include "../renderers/Lighting.hlsli"

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

	uint wireframeMode;
	float3 wireframeColor;

	uint2 heightMapSize;
	float2 heightMapTexelSize;

	float3 WorldSize;
	float padding01;

	float3 SectorCount;
	float padding03;

	float3 CellCount;
	float padding04;

	float3 NodeCount;
	float padding05;

	float3 VertexCount;
	float padding06;

	float3 SectorSize;
	float padding07;

	float3 CellSize;
	float padding08;
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

#include "../GBuffer.hlsli"
#include "../renderers/Normalmapping.hlsli"
#include "../renderers/Materials.hlsli"
#include "../renderers/Fog.hlsli"

float3 normalFromHeights(float4 height)
{
	float3 current = float3(
		0,
		height.x,
		0);
	float3 right = float3(
		heightMapTexelSize.x,
		height.y,
		0);
	float3 bottom = float3(
		0,
		height.z,
		-heightMapTexelSize.y);

	return
		normalize(cross(
			right - current,
			bottom - current));
}

float3 getHeightData(float2 heightUV)
{
	heightUV = float2(heightUV.x, 1.0f - heightUV.y);

	float2 heightTexelPos = heightUV * float2(heightMapSize);
	float2 topLeft = floor(heightTexelPos);
	float2 gatherCenter = topLeft + float2(0.5f, 0.5f);
	float2 posFrac = (heightTexelPos - topLeft) / float2(heightMapSize);

	float a0 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(-1.0f, -1.0f)) / float2(heightMapSize), 0);
	float a1 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(0.0f, -1.0f)) / float2(heightMapSize), 0);
	float a2 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(1.0f, -1.0f)) / float2(heightMapSize), 0);

	float b0 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(-1.0f, 0.0f)) / float2(heightMapSize), 0);
	float b1 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(0.0f, 0.0f)) / float2(heightMapSize), 0);
	float b2 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(1.0f, 0.0f)) / float2(heightMapSize), 0);

	float c0 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(-1.0f, 1.0f)) / float2(heightMapSize), 0);
	float c1 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(0.0f, 1.0f)) / float2(heightMapSize), 0);
	float c2 = heightmap.SampleLevel(tex_sampler, posFrac + (gatherCenter + float2(1.0f, 1.0f)) / float2(heightMapSize), 0);

	// a0, a1, a2
	// b0, b1, b2
	// c0, c1, c2

	float3 n1 = normalFromHeights(float4(a0, a1, b0, b1));
	float3 n2 = normalFromHeights(float4(a1, a2, b1, b2));
	float3 n3 = normalFromHeights(float4(b0, b1, c0, c1));
	float3 n4 = normalFromHeights(float4(b1, b2, c1, c2));
	return (n1 + n2 + n3 + n4) / 4;
}

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
	float occlusion = 1.0f;// ssao.Sample(tex_sampler, screenUV);

	// calculate world position
	float2 inverseSize = float2(1.0f / (float)frameSize.x, 1.0f / (float)frameSize.y);
	float3 inputpos = float3(
		(input.position.x * inverseSize.x) * 2.0f - 1.0f,
		((input.position.y * inverseSize.y) * 2.0f - 1.0f) * -1.0f,
		depthSample);
	float4 ci = mul(cameraInverseProjectionMatrix, float4(inputpos.xyz, 1.0f));
	ci.xyz /= ci.w;
	float3 worldPosition = mul(cameraInverseViewMatrix, float4(ci.xyz, 0.0f)).xyz + cameraWorldSpacePosition;

	TransformHistory transforms = transformHistory[input.instancePointer];
	float4x4 normalTransform = transpose(transforms.inverseTransform);

	float3 normal = getHeightData(objectUV);
	normal = normalize(mul(normalTransform, float4(normal, 0.0f))).xyz;
	float3 tangent = float3(1, 0, 0);

	//float3 normal = normalize(input.normal.xyz);
	//float3 tangent = normalize(input.tangent.xyz);

	// get object material values
	float3 normalSample = normal;
	float4 albedo = float4(colormap.SampleLevel(point_sampler, objectUV, 0).xyz, 1.0f);

#ifdef OPTION_DEBUG
	if (debugMode == DEBUG_MODE_CLUSTERS)
	{
		uint instanceId = input.clusterPointer;
		albedo = float4(
			rand_1_05(float2(instanceId, instanceId)),
			rand_1_05(float2(instanceId + 1, instanceId)),
			rand_1_05(float2(instanceId + 2, instanceId)),
			1.0f);
	}
#endif

	float roughness = 0.999f;
	float metalness = 0.0f;
#ifdef OPTION_DEBUG
	float2 scaledObjectUV = objectUV;

	float albedoMip = 0.0f;
	float albedoMaxMip = 0.0f;

	float roughnessMip = 0.0f;
	float roughnessMaxMip = 0.0f;

	float metalnessMip = 0.0f;
	float metalnessMaxMip = 0.0f;

	float aoMip = 0.0f;
	float aoMaxMip = 0.0f;
#endif
	bool hasNormal = true;

	/*bool hasNormal = getMaterials(
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
	);*/

	// create normal from normal map sample and object normal/tangent
	if (hasNormal)
		normal = normalMap(
			isFrontFace,
			objectUV.x < 0.0,
			normalSample,
			normal,
			tangent);

	

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

	#include "../renderers/EnvironmentIBL.hlsli"

	color = applyFog(color, length(cameraWorldSpacePosition - worldPosition), cameraWorldSpacePosition, V);

	PSOutput output;

	float2 cur = (input.mvPosCurrent.xy / input.mvPosCurrent.w) * 0.5 + 0.5;
	float2 pre = (input.mvPosPrevious.xy / input.mvPosPrevious.w) * 0.5 + 0.5;
	cur.y = 1.0f - cur.y;
	pre.y = 1.0f - pre.y;
	output.motion = cur - pre;

	float4 outputColor;
	#include "../renderers/ColorOutput.hlsli"
	if (wireframeMode)
		output.color = float4(wireframeColor, 1.0f);
	else
		output.color = outputColor;

	return output;
}
