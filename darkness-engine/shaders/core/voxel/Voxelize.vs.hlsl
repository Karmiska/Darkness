
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
Buffer<float2> uv;

StructuredBuffer<VertexScale> scales;
StructuredBuffer<ClusterInstanceData> clusters;
StructuredBuffer<ClusterData> clusterData;
StructuredBuffer<TransformHistory> transformHistory;

cbuffer CameraMatrixes
{
	float4x4 viewProjectionMatrix;
};

struct VSOutput
{
	float4 position         : SV_Position0;
	float4 normal           : NORMAL0;
	float3 uv				: TEXCOORD0;
	uint instancePointer    : BLENDINDICES0;
};

VSOutput main(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
	const float4 invalid = float4(
		asfloat(0x7f800001),
		asfloat(0x7f800001),
		asfloat(0x7f800001),
		asfloat(0x7f800001));

	ClusterInstanceData cInstanceData = clusters[instanceId];
	ClusterData cdata = clusterData[cInstanceData.clusterPointer];

	VSOutput output;
	if (id < cdata.indexCount)
	{

		uint clusterIndex = baseIndexes[cdata.indexPointer + id];

		uint vertexDataIndex = cdata.vertexPointer + clusterIndex;
		uint uvDataIndex = cdata.vertexPointer + clusterIndex;

		TransformHistory transforms = transformHistory[cInstanceData.instancePointer];
		float4x4 normalTransform = transpose(transforms.inverseTransform);

		float3 vertex = unpackVertex(vertices[vertexDataIndex], scales[cInstanceData.instancePointer]);

		output.position = mul(mul(viewProjectionMatrix, transforms.transform), float4(vertex, 1));
		output.normal = normalize(mul(normalTransform, float4(unpackNormalOctahedron(normals[vertexDataIndex]), 0.0f)));
		output.instancePointer = cInstanceData.instancePointer;

		output.uv.xy = flipVerticalUV(uv[uvDataIndex]);
		output.uv.z = output.position.z;

		return output;

	}

	output.position = invalid;
	output.normal = invalid;
	output.instancePointer = 0;
	return output;
}
