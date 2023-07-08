#include "../../shared_types/ClusterInstanceData.hlsli"
#include "../../shared_types/ClusterData.hlsli"
#include "../../shared_types/TransformHistory.hlsli"
#include "../../shared_types/InstanceMaterial.hlsli"
#include "../../shared_types/SubMeshUVLod.hlsli"
#include "../../shared_types/BoundingBox.hlsli"
#include "../../shared_types/VertexScale.hlsli"
#include "../../shared_types/DebugModes.hlsli"
#include "../../GBuffer.hlsli"
#include "../../VertexPacking.hlsli"
#include "../../Common.hlsli"

Buffer<uint2> vertices;
Buffer<float2> normals;
Buffer<float2> tangents;
Buffer<float2> uv;

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
	float4x4 viewProjectionMatrix;
	float4x4 previousViewProjectionMatrix;
	float4x4 jitterViewProjectionMatrix;

	uint debugMode;
	uint3 padding;
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

VSOutput main(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
#include "../VertexProcessing.hlsli"
}
