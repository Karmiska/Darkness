
static const float3 cubeVertices[24] =
{
	float3(-1.0, +1.0, -1.0),	// top		0
	float3(-1.0, +1.0, +1.0),
	float3(+1.0, +1.0, +1.0),
	float3(+1.0, +1.0, -1.0),

	float3(-1.0, -1.0, +1.0),	// bottom	4
	float3(-1.0, -1.0, -1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, -1.0, +1.0),

	float3(-1.0, +1.0, +1.0),	// front	8
	float3(-1.0, -1.0, +1.0),
	float3(+1.0, -1.0, +1.0),
	float3(+1.0, +1.0, +1.0),

	float3(+1.0, +1.0, -1.0),	// back		12
	float3(+1.0, -1.0, -1.0),
	float3(-1.0, -1.0, -1.0),
	float3(-1.0, +1.0, -1.0),

	float3(+1.0, +1.0, +1.0),	// right	16
	float3(+1.0, -1.0, +1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, +1.0, -1.0),

	float3(-1.0, +1.0, -1.0),	// left		20
	float3(-1.0, -1.0, -1.0),
	float3(-1.0, -1.0, +1.0),
	float3(-1.0, +1.0, +1.0)
};

static const uint cubeIndexes[36] =
{
	0, 1, 2, 2, 3, 0,		// top
	4, 5, 6, 6, 7, 4,		// bottom
	8, 9, 10, 10, 11, 8,	// front
	12, 13, 14, 14, 15, 12,	// back
	16, 17, 18, 18, 19, 16,	// right
	20, 21, 22, 22, 23, 20
};

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
StructuredBuffer<BoundingBox> clusterBoundingBoxes;

cbuffer CameraMatrixes
{
	float4x4 viewProjectionMatrix;
	float4x4 jitterViewProjectionMatrix;
};

struct VSOutput
{
	float4 position     : SV_Position0;
	float4 color		: COLOR0;
};

float3 getVertexFromBoundingBox(BoundingBox bb, uint id)
{
	uint vertex = cubeIndexes[id];
	if (vertex < 4)
	{
		if (vertex == 0) return float3(bb.min.x, bb.max.y, bb.min.z);
		else if (vertex == 1) return float3(bb.min.x, bb.max.y, bb.max.z);
		else if (vertex == 2) return float3(bb.max.x, bb.max.y, bb.max.z);
		else if (vertex == 3) return float3(bb.max.x, bb.max.y, bb.min.z);
		else return float3(0, 0, 0);
	}
	else if (vertex < 8)
	{
		if (vertex == 4) return float3(bb.min.x, bb.min.y, bb.max.z);
		else if (vertex == 5) return float3(bb.min.x, bb.min.y, bb.min.z);
		else if (vertex == 6) return float3(bb.max.x, bb.min.y, bb.min.z);
		else if (vertex == 7) return float3(bb.max.x, bb.min.y, bb.max.z);
		else return float3(0, 0, 0);
	}
	else if (vertex < 12)
	{
		if (vertex == 8)  return float3(bb.min.x, bb.max.y, bb.max.z);
		else if (vertex == 9)  return float3(bb.min.x, bb.min.y, bb.max.z);
		else if (vertex == 10) return float3(bb.max.x, bb.min.y, bb.max.z);
		else if (vertex == 11) return float3(bb.max.x, bb.max.y, bb.max.z);
		else return float3(0, 0, 0);
	}
	else if (vertex < 16)
	{
		if (vertex == 12) return float3(bb.max.x, bb.max.y, bb.min.z);
		else if (vertex == 13) return float3(bb.max.x, bb.min.y, bb.min.z);
		else if (vertex == 14) return float3(bb.min.x, bb.min.y, bb.min.z);
		else if (vertex == 15) return float3(bb.min.x, bb.max.y, bb.min.z);
	}
	else if (vertex < 20)
	{
		if (vertex == 16) return float3(bb.max.x, bb.max.y, bb.max.z);
		else if (vertex == 17) return float3(bb.max.x, bb.min.y, bb.max.z);
		else if (vertex == 18) return float3(bb.max.x, bb.min.y, bb.min.z);
		else if (vertex == 19) return float3(bb.max.x, bb.max.y, bb.min.z);
		else return float3(0, 0, 0);
	}
	else
	{
		if (vertex == 20) return float3(bb.min.x, bb.max.y, bb.min.z);
		else if (vertex == 21) return float3(bb.min.x, bb.min.y, bb.min.z);
		else if (vertex == 22) return float3(bb.min.x, bb.min.y, bb.max.z);
		else if (vertex == 23) return float3(bb.min.x, bb.max.y, bb.max.z);
		else return float3(0, 0, 0);
	}
	return float3(0, 0, 0);
}

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
		/*uint clusterIndex = baseIndexes[cdata.indexPointer + id];

		uint vertexDataIndex = cdata.vertexPointer + clusterIndex;
		uint uvDataIndex = cInstanceData.uvPointer + clusterIndex;*/

		TransformHistory transforms = transformHistory[cInstanceData.instancePointer];
		//float4x4 normalTransform = transpose(transforms.inverseTransform);
		//
		//float3 vertex = unpackVertex(vertices[vertexDataIndex], scales[cInstanceData.instancePointer]);

		BoundingBox bb = clusterBoundingBoxes[cInstanceData.clusterPointer];
		float3 vertex = getVertexFromBoundingBox(bb, id);

		output.position = mul(mul(viewProjectionMatrix, transforms.transform), float4(vertex, 1));
		output.color = float4(1, 0, 0, 1);

		return output;
	}

	output.position = invalid;
	output.color = float4(0, 0, 0, 0);

	return output;
}
