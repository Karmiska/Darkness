
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/ClusterData.hlsli"
#include "../shared_types/ClusterExecuteIndirect.hlsli"

Buffer<uint>									inputClusterCount;
RWBuffer<uint>									outputDrawCount;
RWStructuredBuffer<ClusterExecuteIndirectArgs>  clusterRendererExecuteIndirectArguments;

cbuffer CreateDebugBoundingBoxDrawArgsConstants
{
	uint clusterSize;
	uint3 padding;
};

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	float maxClusters = inputClusterCount[0];
	uint threads = maxClusters * clusterSize;

	clusterRendererExecuteIndirectArguments[0].IndexCountPerInstance = clusterSize;
	clusterRendererExecuteIndirectArguments[0].InstanceCount = maxClusters;
	clusterRendererExecuteIndirectArguments[0].StartIndexLocation = 0;
	clusterRendererExecuteIndirectArguments[0].BaseVertexLocation = 0;
	clusterRendererExecuteIndirectArguments[0].StartInstanceLocation = 0;

	outputDrawCount[0] = 1;
}
