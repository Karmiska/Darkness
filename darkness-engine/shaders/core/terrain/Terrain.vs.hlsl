
struct Vertex
{
	float x;
	float y;
	float z;
};

StructuredBuffer<Vertex> vertices;
Texture2D<float> heightmap;
Texture2D<float4> colormap;

sampler heightMapSampler;
sampler colorMapSampler;

cbuffer CameraMatrixes
{
	float4x4 viewProjectionMatrix;

	float3 cameraWorldPosition;
	float heightBias;

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

struct VSOutput
{
	float4 position         : SV_Position0;
	float4 worldPosition	: POSITION0;
	float4 color			: TEXCOORD0;
};

float rand_1_05(in float2 uv)
{
	float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	return abs(noise.x + noise.y) * 0.5;
}

#include "TerrainCommon.hlsli"

VSOutput main(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
	VSOutput output;

	// camera position snaps on cells
	int3 cameraSnapPosition = int3(floor(cameraWorldPosition / CellSize)) * CellSize;

	// one instance equals one Cell (one Cell is technically a strip of indexes, akin to cluster)
	uint cellsInSector = CellCount.x * CellCount.z;
	uint sectorId = instanceId / cellsInSector;
	if (sectorId == 0)
	{
		// sector 0
		uint cellInSector = instanceId - (sectorId * cellsInSector);
		uint zlocation = cellInSector / CellCount.x;
		uint xlocation = cellInSector - (zlocation * CellCount.x);

		float3 vertexWorldPosition = getVertexWorldPosition(
			float3(vertices[id].x, vertices[id].y, vertices[id].z), 
			NodeCount, SectorSize, CellSize, cameraSnapPosition, uint2(xlocation, zlocation), float3(0, 0, 0));
		float2 posUV;
		float4 height;
		float interpolatedHeight = getHeightData(WorldSize, vertexWorldPosition, posUV, height);
		vertexWorldPosition.y = WorldPosition.y + (WorldSize.y * interpolatedHeight);
		vertexWorldPosition.y += heightBias;

		output.position = mul(viewProjectionMatrix, float4(vertexWorldPosition, 1.0f));
		output.worldPosition = float4(vertexWorldPosition, 1.0f);

		float3 col = colormap.SampleLevel(colorMapSampler, posUV, 0).xyz;

		// Debug output cell colors
		//output.color = float4(
		//	rand_1_05(float2(zlocation, xlocation)),
		//	rand_1_05(float2(zlocation + 1, xlocation)),
		//	rand_1_05(float2(zlocation + 2, xlocation)),
		//	1.0f);

		output.color = float4(float2(1.0f - posUV.x, posUV.y), 0, 1);
	}
	else
	{
		output.position = mul(viewProjectionMatrix, float4(
			vertices[id].x,
			vertices[id].y,
			vertices[id].z,
			1.0f));
	}
	return output;
}
