
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/ClusterData.hlsli"
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../shared_types/SubMeshUVLod.hlsli"
#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/VertexScale.hlsli"
#include "../GBuffer.hlsli"
#include "../VertexPacking.hlsli"
#include "../Common.hlsli"

Buffer<uint> baseIndexes;
Buffer<uint2> vertices;
StructuredBuffer<VertexScale> scales;
StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;

Texture2D<float> heightmap;
sampler heightMapSampler;

cbuffer CameraMatrixes
{
	float4x4 viewProjectionMatrix;

	float3 cameraWorldPosition;
	float heightBias;

	uint2 heightMapSize;
	float2 heightMapTexelSize;

	float3 WorldSize;
	uint clusterStartPtr;

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

#include "TerrainCommon.hlsli"

float4 main(uint id : SV_VertexID, uint instanceId : SV_InstanceID) : SV_Position0
{
	ClusterInstanceData cInstanceData = clusters[instanceId];
	uint cellInstanceId = cInstanceData.clusterPointer - clusterStartPtr;

	TransformHistory transforms = transformHistory[cInstanceData.instancePointer];

	// camera position snaps on cells
	float3 worldPosition = mul(transforms.transform, float4(cameraWorldPosition, 1.0f)).xyz;
	int3 cameraSnapPosition = getCameraSnapPosition(worldPosition, CellSize);

	// one instance equals one Cell (one Cell is technically a strip of indexes, akin to cluster)
	uint cellsInSector = CellCount.x * CellCount.z;
	uint sectorId = cellInstanceId / cellsInSector;
	if (sectorId == 0)
	{
		// sector 0
		uint cellInSector = cellInstanceId - (sectorId * cellsInSector);
		uint zlocation = cellInSector / CellCount.x;
		uint xlocation = cellInSector - (zlocation * CellCount.x);

		float3 vertexWorldPosition = getVertexWorldPosition(
			getVertex(id),
			NodeCount, SectorSize, CellSize, cameraSnapPosition, uint2(xlocation, zlocation), float3(0, 0, 0));
		float2 posUV;
		float4 height;
		float interpolatedHeight = getHeightData(WorldSize, vertexWorldPosition, posUV, height);
		vertexWorldPosition.y = WorldSize.y * interpolatedHeight;
		vertexWorldPosition.y += heightBias;

		return mul(mul(viewProjectionMatrix, transforms.transform), float4(vertexWorldPosition, 1));
	}
	else
	{
		float3 vert = getVertex(id);
		return mul(viewProjectionMatrix, float4(
			vert.x,
			vert.y,
			vert.z,
			1.0f));
	}
}
