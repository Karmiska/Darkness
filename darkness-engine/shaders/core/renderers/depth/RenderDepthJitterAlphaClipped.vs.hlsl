
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
Buffer<float2> uv;
StructuredBuffer<VertexScale> scales;
StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;
StructuredBuffer<InstanceMaterial> instanceMaterials;

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

cbuffer RenderDepthJitterAlphaClippedVSConstants
{
	float4x4 jitterViewProjectionMatrix;
};

struct VSOutput
{
	float4 position     : SV_Position0;
	float2 uv			: TEXCOORD0;
	uint albedoMaterial	: BLENDINDICES0;
};

VSOutput main(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
	if (id != 0xffffffff)
	{
		uint clusterId = (id & 0xffff0000) >> 16;
		uint clusterIndex = id & 0x0000ffff;
		ClusterInstanceData cInstanceData = clusters[rootConstants.clusterStart + clusterId];
		ClusterData cdata = clusterData[cInstanceData.clusterPointer];

		TransformHistory transforms = transformHistory[cInstanceData.instancePointer];
		uint vertexDataIndex = cdata.vertexPointer + clusterIndex;
		uint uvDataIndex = cdata.vertexPointer + clusterIndex;
		float3 vertex = unpackVertex(vertices[vertexDataIndex], scales[cInstanceData.instancePointer]);

		InstanceMaterial material = instanceMaterials[cInstanceData.instancePointer];

		VSOutput output;
		output.position = mul(mul(jitterViewProjectionMatrix, transforms.transform), float4(vertex, 1));
		output.albedoMaterial = material.albedo;
		output.uv = flipVerticalUV(uv[uvDataIndex]);
		return output;
	}
	else
	{
		VSOutput output;
		output.position = float4(
			asfloat(0x7f800001),
			asfloat(0x7f800001),
			asfloat(0x7f800001),
			asfloat(0x7f800001));
		output.albedoMaterial = 0;
		output.uv = float2(0.0f, 0.0f);
		return output;
	}
}
