
#include "../Common.hlsli"
#include "../PhysicallyBasedRendering.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../shared_types/DebugModes.hlsli"

TextureCube<float4> environmentIrradianceCubemap;
Texture2D<float4> environmentIrradiance;
TextureCube<float4> environmentSpecular;
Texture2D<float4> environmentBrdfLut;
Texture2D<float4> colormap;
Texture2D<float> ssao;
Texture2D<float> heightmap;

#include "../renderers/Lighting.hlsli"

struct PSInput
{
	float4 position         : SV_Position0;
	float4 worldPosition	: POSITION0;
	float4 color			: TEXCOORD0;
};

cbuffer TerrainPSConstants
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

	float3 WorldPosition;
	float padding02;

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

float3 getHeightData(float3 worldPosition, float3 worldSize, float3 vertexWorldPosition)
{
	float2 heightMapLocation = (vertexWorldPosition - worldPosition).xz;
	float2 heightUV = heightMapLocation / worldSize.xz;
	/*float2 heightTexelPos = heightUV * float2(heightMapSize);
	float2 topLeft = floor(heightTexelPos);

	// interpolated height
	float4 c0 = heightmap.Gather(point_sampler, (topLeft + float2(1.5f, 1.5f)) / float2(heightMapSize));
	float4 c1 = heightmap.Gather(point_sampler, (topLeft + float2(2.5f, 2.5f)) / float2(heightMapSize));
	float c00 = heightmap.Sample(point_sampler, (topLeft + float2(3.0f, 0.0f)) / float2(heightMapSize));
	float c01 = heightmap.Sample(point_sampler, (topLeft + float2(0.0f, 3.0f)) / float2(heightMapSize));

	float3 n1 = normalFromHeights(c0);
	//return n1;
	float3 n2 = normalFromHeights(c1);
	//return n2;
	float3 n3 = normalFromHeights(float4(c0.y, c00, c0.w, c1.y));
	return n3;
	float3 n4 = normalFromHeights(float4(c01, c1.z, c0.z, c0.w));
	return n4;*/

	float2 heightTexelPos = heightUV * float2(heightMapSize);
	float2 topLeft = floor(heightTexelPos);
	float2 gatherCenter = topLeft + float2(0.5f, 0.5f);
	float2 posFrac = (heightTexelPos - topLeft) / float2(heightMapSize);
	//posUV = gatherCenter / float2(heightMapSize);

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

	//float3 n1 = normalFromHeights(heightmap.Gather(tex_sampler, heightUV));
	//float3 n2 = normalFromHeights(heightmap.Gather(tex_sampler, heightUV + float2(heightMapTexelSize.x, 0.0f)));
	//float3 n3 = normalFromHeights(heightmap.Gather(tex_sampler, heightUV + float2(0.0f, heightMapTexelSize.y)));
	//float3 n4 = normalFromHeights(heightmap.Gather(tex_sampler, heightUV + float2(heightMapTexelSize.x, heightMapTexelSize.y)));

	//return (n1 + n2 + n3 + n4) / 4;
}

float4 main(PSInput input) : SV_Target
{
	float3 worldPosition = input.worldPosition.xyz;
	float3 albedo = colormap.SampleLevel(point_sampler, input.color.xy, 0).xyz; // float4(input.color.x, input.color.x, input.color.x, 1);
	float2 screenUV = input.position.xy / resolution.xy;
	float occlusion = 1.0f;// ssao.Sample(tex_sampler, screenUV);

	float3 normal = getHeightData(WorldPosition, WorldSize, worldPosition);

	// setup lighting values
	float metalness = 0.0f;
	float roughness = 0.999f;
	float3 V = normalize(cameraWorldSpacePosition - worldPosition);
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, input.color.xyz, metalness);
	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	uint drawnLights = performLighting(
		worldPosition,
		cameraWorldSpacePosition,
		normal, V,
		albedo,
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

	float4 outputColor;
#include "../renderers/ColorOutput.hlsli"

	if (wireframeMode == 0)
		return float4(outputColor.xyz, 1.0);
		//return float4(normal, 1.0);
	else
		return float4(wireframeColor, 1.0);
}
