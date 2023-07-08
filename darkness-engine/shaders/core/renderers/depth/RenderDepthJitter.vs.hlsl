
#include "../../shared_types/ClusterInstanceData.hlsli"
#include "../../shared_types/ClusterData.hlsli"
#include "../../shared_types/TransformHistory.hlsli"
#include "../../shared_types/InstanceMaterial.hlsli"
#include "../../shared_types/SubMeshUVLod.hlsli"
#include "../../shared_types/BoundingBox.hlsli"
#include "../../shared_types/VertexScale.hlsli"
#include "../../GBuffer.hlsli"
#include "../../VertexPacking.hlsli"
#include "../../Common.hlsli"

Buffer<uint2> vertices;
StructuredBuffer<VertexScale> scales;
StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;

struct RootConstants
{
	uint clusterStart;
	uint IndexCountPerInstance;
	uint InstanceCount;
	uint StartIndexLocation;
	int BaseVertexLocation;
	uint StartInstanceLocation;
};
ConstantBuffer<RootConstants> rootConstants;

cbuffer CameraMatrixes
{
	float4x4 jitterViewProjectionMatrix;
};

float4 main(uint id : SV_VertexID) : SV_Position0
{
	if (id != 0xffffffff)
	{
		uint clusterId = (id & 0xffff0000) >> 16;
		uint clusterIndex = id & 0x0000ffff;
		ClusterInstanceData cInstanceData = clusters[rootConstants.clusterStart + clusterId];
		ClusterData cdata = clusterData[cInstanceData.clusterPointer];

		TransformHistory transforms = transformHistory[cInstanceData.instancePointer];
		uint vertexDataIndex = cdata.vertexPointer + clusterIndex;
		float3 vertex = unpackVertex(vertices[vertexDataIndex], scales[cInstanceData.instancePointer]);

		return mul(mul(jitterViewProjectionMatrix, transforms.transform), float4(vertex, 1));
	}
	else
	{
		return float4(
			asfloat(0x7f800001),
			asfloat(0x7f800001),
			asfloat(0x7f800001),
			asfloat(0x7f800001));
	}
}
