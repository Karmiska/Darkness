
#include "../shared_types/BoundingBox.hlsli"
#include "../shared_types/TransformHistory.hlsli"

RWStructuredBuffer<BoundingBox>     subMeshBoundingBoxes;
RWStructuredBuffer<BoundingBox>     clusterBoundingBoxes;
StructuredBuffer<TransformHistory>	transformHistory;

Texture2D<float> heightmap;

sampler heightMapSampler;

cbuffer TerrainRefreshConstants
{
	float3 cameraWorldPosition;
	uint wholeVertexCount;
	
	uint2 heightMapSize;
	float2 heightMapTexelSize;

	float3 WorldSize;
	uint vertexPerCluster;

	float3 SectorCount;
	uint subMeshBoundingBoxIndex;

	float3 CellCount;
	uint clusterStartPtr;

	float3 NodeCount;
	uint terrainInstancePtr;

	float3 VertexCount;
	float padding06;

	float3 SectorSize;
	float padding07;

	float3 CellSize;
	float padding08;
};

#include "TerrainCommon.hlsli"

groupshared uint minHeight;
groupshared uint maxHeight;

[numthreads(121, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
	if (groupThreadID.x == 0)
	{
		minHeight = 4294967295;
		maxHeight = 0;
	}

	GroupMemoryBarrierWithGroupSync();

	int instanceId = groupID.x;// dispatchThreadID.x / vertexPerCluster;
	int id = groupThreadID.x;

	// one instance equals one Cell (one Cell is technically a strip of indexes, akin to cluster)
	uint cellsInSector = CellCount.x * CellCount.z;
	uint sectorId = instanceId / cellsInSector;

	uint cellInSector = instanceId - (sectorId * cellsInSector);
	uint zlocation = cellInSector / CellCount.x;
	uint xlocation = cellInSector - (zlocation * CellCount.x);

	TransformHistory transforms = transformHistory[terrainInstancePtr];

	// camera position snaps on cells
	float3 worldPosition = mul(transforms.transform, float4(cameraWorldPosition, 1.0f)).xyz;
	int3 cameraSnapPosition = getCameraSnapPosition(worldPosition, CellSize);

	//if (dispatchThreadID.x < wholeVertexCount)
	{
		if (sectorId == 0)
		{
			// sector 0
			

			float3 vertexWorldPosition = getVertexWorldPosition(
				getVertex(id),
				NodeCount, SectorSize, CellSize, cameraSnapPosition, uint2(xlocation, zlocation), float3(0, 0, 0));
			float2 posUV;
			float4 height;
			float interpolatedHeight = 0.0f;
			if((vertexWorldPosition.x >= 0 && vertexWorldPosition.x < WorldSize.x) &&
				(vertexWorldPosition.z >= 0 && vertexWorldPosition.z < WorldSize.z))
				interpolatedHeight = getHeightData(WorldSize, vertexWorldPosition, posUV, height);
			vertexWorldPosition.y = WorldSize.y * interpolatedHeight;

			uint scaledHeight = 4294967295.0f * (vertexWorldPosition.y / WorldSize.y);
			InterlockedMin(minHeight, scaledHeight);
			InterlockedMax(maxHeight, scaledHeight);
		}
	}

	GroupMemoryBarrierWithGroupSync();

	if (groupThreadID.x == 0)
	{
		// every groups first thread will do the min maxing
		uint2 cellLocation = uint2(xlocation, zlocation);
		float3 sectorWorldPosition = float3(cameraSnapPosition)-float3(SectorSize.x / 2.0f, 0.0f, SectorSize.z / 2.0f);
		float3 cellWorldPosition = sectorWorldPosition + float3(cellLocation.x * CellSize.x, 0.0f, cellLocation.y * CellSize.z);

		float minValue = (((float)minHeight / 4294967295.0f) * WorldSize.y);
		float maxValue = (((float)maxHeight / 4294967295.0f) * WorldSize.y);

		BoundingBox box;
		/*if ((!((cellWorldPosition.x < 0) || 
			(cellWorldPosition.x >= WorldSize.x) ||
			(cellWorldPosition.z < 0) ||
			(cellWorldPosition.z >= WorldSize.z))) ||
		(abs(maxValue - minValue)-0.0001f < 0.0f))*/
#ifdef OPTION_DEBUG
#else
		if (abs(maxValue - minValue) - 0.0001f < 0.0f)
		{
			box.min = float3(
				asfloat(0x7f800001),
				asfloat(0x7f800001),
				asfloat(0x7f800001));
		
			box.max = float3(
				asfloat(0x7f800001),
				asfloat(0x7f800001),
				asfloat(0x7f800001));
		}
		else
#endif
		{
			box.min = float3(
				cellWorldPosition.x,
				minValue,
				cellWorldPosition.z);

			box.max = float3(
				cellWorldPosition.x + CellSize.x,
				maxValue,
				cellWorldPosition.z + CellSize.z);
		}
		box.padding1 = 0;
		box.padding2 = 0;

		clusterBoundingBoxes[clusterStartPtr + groupID.x] = box;
	}

	if (dispatchThreadID.x == 0)
	{
		BoundingBox box;
		box.min = float3(0.0f, 0.0f, 0.0f);
		box.max = WorldSize;
		box.padding1 = 0;
		box.padding2 = 0;

		subMeshBoundingBoxes[subMeshBoundingBoxIndex] = box;
	}
}
