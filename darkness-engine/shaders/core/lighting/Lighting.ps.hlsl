
#include "../Common.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../shared_types/DebugModes.hlsli"
#include "../PhysicallyBasedRendering.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

TextureCube<float4> environmentIrradianceCubemap;
Texture2D<float4> environmentIrradiance;
TextureCube<float4> environmentSpecular;
Texture2D<float4> environmentBrdfLut;

Texture2D<float> ssao;
Texture2D<float4> ssr;

#include "../renderers/Lighting.hlsli"

Texture2D<float> depth;

cbuffer ConstData
{
    // 16 floats
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
};

sampler tex_sampler;
sampler tri_sampler;
sampler depth_sampler;
sampler point_sampler;

float3 sampleEnvironment(float3 direction, float lod)
{
    float3 inverseDirection = float3(direction.x, direction.y, -direction.z);
    if (hasEnvironmentIrradianceCubemap)
        return environmentIrradianceCubemap.SampleLevel(tri_sampler, inverseDirection, lod).xyz;
    else if(hasEnvironmentIrradianceEquirect)
        return environmentIrradiance.SampleLevel(tri_sampler, envMapEquirect(direction), lod).xyz;
    else 
        return float3(0.0f, 0.0f, 0.0f);
}

#include "../GBuffer.hlsli"
#include "../renderers/GBufferMaterials.hlsli"
#include "../renderers/Fog.hlsli"
#include "../renderers/PhysicalCamera.hlsli"

Texture3D<uint> voxels[];
Texture3D<float3> voxelNormals;
Buffer<uint> voxelColorgrid;

int3 voxelGridCoordinate(float3 pos, int mip)
{
	//pos /= (mip + 1);
	//return int3(pos.x, pos.y, pos.z);
	return int3(pos.x, pos.y, pos.z) >> mip;
}

bool voxelGridHit(float3 pos, int mip)
{
    return voxels[mip][voxelGridCoordinate(pos, mip)];
}

#include "../voxel/VoxelDataCommon.hlsli"


// ACCURATE
/*bool gridMarch(float3 origin, float3 direction, float3 normal, out int3 hit)
{
	direction *= -1.0f;

	// fix this
	const float maxDistance = voxelGridSize.x * voxelGridSize.y;
	float distance = 0;
	int steps = 1500;

	origin += (normal * -1 * 3);
	while (distance < maxDistance && steps > 0)
	{
		float3 pnt = origin + direction * distance;

		if (!insideVoxelGrid(pnt))
			return false;

		if (voxelGridHit(pnt, 0))
		{
			hit = int3(pnt.x, pnt.y, pnt.z);
			return true;
		}

		float3 deltas = (step(0, direction) - frac(pnt)) / direction + 0.0001f;
		distance += max(min(min(deltas.x, deltas.y), deltas.z), FLT_EPS);

		--steps;
	}
	return false;
}*/

float3 intersectPoint(
	float3 pnt,
	float3 direction,
	float3 planePoint,
	float3 planeNormal)
{
	float3 diff = pnt - planePoint;
	float prod1 = dot(diff, planeNormal);
	float prod2 = dot(direction, planeNormal);
	float prod3 = prod1 / (prod2 + FLT_EPS);
	return pnt - direction * prod3;
}

float intersect(
	float3 position,
	float3 direction,
	float3 plane_normal,
	float3 plane_point)
{
	float3 f = frac(position);

	

	float3 ix = intersectPoint(f, direction, float3( plane_point.x, 0.0f, 0.0f ), float3( plane_normal.x, 0.0f, 0.0f ));
	float3 iy = intersectPoint(f, direction, float3( 0.0f, plane_point.y, 0.0f ), float3( 0.0f, plane_normal.y, 0.0f ));
	float3 iz = intersectPoint(f, direction, float3( 0.0f, 0.0f, plane_point.z ), float3( 0.0f, 0.0f, plane_normal.z ));

	return min(min(length(ix - f), length(iy - f)), length(iz - f));
}

float3 toMip(float3 pnt, int mip)
{
	return (pnt / voxelDepth) * ((int)voxelDepth >> mip);
}

float3 fromMip(float3 pnt, int mip)
{
	return (pnt / ((int)voxelDepth >> mip)) * voxelDepth;
}

// FAST
bool gridMarch(float3 origin, float3 direction, float3 normal, out int3 hit)
{
	direction *= -1.0f;

	// fix this
	const float maxDistance = voxelGridSize.x * voxelGridSize.y;
	float distance = 0;
	int steps = 280;
	
	bool fastAdvance = true;
	const int fastMip = 2;
	float fastInc = (voxelGridSize.x / voxelDepth) * ((uint)1 << (uint)fastMip) * 15;
	
	origin += (normal * -1 * 14);

	float3 plane_normal = float3(direction.x >= 0.0f ? -1.0f : 1.0f, direction.y >= 0.0f ? -1.0f : 1.0f, direction.z >= 0.0f ? -1.0f : 1.0f);
	float3 plane_point = float3(direction.x >= 0.0f ? 1.0f : 0.0f, direction.y >= 0.0f ? 1.0f : 0.0f, direction.z >= 0.0f ? 1.0f : 0.0f);

	while (distance < maxDistance && steps > 0)
	{
		float3 pnt = origin + direction * distance;

		if (!insideVoxelGrid(pnt))
			return false;

		if (fastAdvance)
		{
			if (voxelGridHit(pnt, fastMip))
			{
				fastAdvance = false;
				distance -= fastInc;
			}
			else distance += fastInc;
		}
		
		if (!fastAdvance)
		{
			if (voxelGridHit(pnt, 0))
			{
				hit = int3(pnt.x, pnt.y, pnt.z);
				return true;
			}
			else
				distance += intersect(pnt, direction, plane_normal, plane_point) + 0.001f;
		}

		--steps;
	}
	return false;
}

float4 main(PSInput input) : SV_Target
{
    // screenUV = 0.0 - 1.0 xy UV
	// gUV = 0 - 1920/1080 uint screen UV
	// objectUV = 0.0 - 1.0 xy UV to object UV
    float2 screenUV = input.uv;
	float depthSample = depth.Sample(depth_sampler, screenUV);
	//clip(depthSample - 0.0001);

	uint2 gUV = uint2((uint)input.position.x, (uint)input.position.y);
	uint instanceId = gbufferInstanceId[gUV];
	float2 objectUV = (float2)gbufferUV[gUV] / 65535.0f;

	// get screen space ambient occlusion
	float occlusion = ssao.Sample(tex_sampler, screenUV);

#ifdef OPTION_VOXELREFLECTIONS
#endif

	// get object material values
	float3 normal = unpackNormalOctahedron(gbufferNormals.Sample(point_sampler, screenUV));
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
	getMaterialsPartialDerivatives(
		instanceId,
		objectUV,
		gUV,
		albedo,
		roughness,
		metalness,
		occlusion
#ifdef OPTION_DEBUG
		, scaledObjectUV
        , albedoMip
        , albedoMaxMip
        , roughnessMip
        , roughnessMaxMip
        , metalnessMip
        , metalnessMaxMip
        , aoMip
        , aoMaxMip
#endif
	);

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
		gUV,
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
    

	float3 ssrSample = ssr.Load(uint3(gUV / 2.0f, 0)).xyz;

	#include "../renderers/EnvironmentIBL.hlsli"

    #include "../renderers/VoxelGI.hlsli"

	color = applyFog(color, length(cameraWorldSpacePosition - worldPosition), cameraWorldSpacePosition, V);

	float4 outputColor;
	#include "../renderers/ColorOutput.hlsli"

    outputColor = applyPhysicalCamera(outputColor);

	return outputColor;
}
