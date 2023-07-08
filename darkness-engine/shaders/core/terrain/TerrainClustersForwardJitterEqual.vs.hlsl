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
Buffer<float2> normals;
Buffer<float2> tangents;
Buffer<float2> uv;

StructuredBuffer<VertexScale> scales;
StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;

Texture2D<float> heightmap;
sampler heightMapSampler;

cbuffer CameraMatrixes
{
	float4x4 viewProjectionMatrix;
	float4x4 previousViewProjectionMatrix;
	float4x4 jitterViewProjectionMatrix;

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

struct VSOutput
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

#include "TerrainCommon.hlsli"

VSOutput main(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
	VSOutput output;

	const float4 invalid = float4(
		asfloat(0x7f800001),
		asfloat(0x7f800001),
		asfloat(0x7f800001),
		asfloat(0x7f800001));

	ClusterInstanceData cInstanceData = clusters[instanceId];
	ClusterData cdata = clusterData[cInstanceData.clusterPointer];

	uint cellInstanceId = cInstanceData.clusterPointer - clusterStartPtr;

	if (id < cdata.indexCount)
	{
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

			// =========================================================================
			
			float3 vertex = vertexWorldPosition;

			output.position = mul(mul(jitterViewProjectionMatrix, transforms.transform), float4(vertex, 1));
			output.mvPosCurrent = mul(mul(viewProjectionMatrix, transforms.transform), float4(vertex, 1));
			output.mvPosPrevious = mul(mul(previousViewProjectionMatrix, transforms.previousTransform), float4(vertex, 1));
			output.normal = float4(0, 1, 0, 0);// normalize(mul(normalTransform, float4(unpackNormalOctahedron(normals[vertexDataIndex]), 0.0f)));
			output.tangent = float4(0, 1, 0, 0);//normalize(mul(normalTransform, float4(unpackNormalOctahedron(tangents[vertexDataIndex]), 0.0f)));
			output.instancePointer = cInstanceData.instancePointer;
#ifdef OPTION_DEBUG
			output.clusterPointer = cInstanceData.clusterPointer;
#endif

			float2 uu = float2(posUV.x, 1.0 - posUV.y);
			output.normal.w = uu.x;
			output.tangent.w = uu.y;
			// =========================================================================

			return output;
		}
	}

	output.position = invalid;
	output.mvPosCurrent = invalid;
	output.mvPosPrevious = invalid;
	output.normal = invalid;
	output.tangent = invalid;
	output.instancePointer = 0;
#ifdef OPTION_DEBUG
	output.clusterPointer = 0;
#endif
	return output;
}
