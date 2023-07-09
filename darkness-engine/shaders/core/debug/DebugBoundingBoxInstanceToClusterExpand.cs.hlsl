
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/FrustumCullingOutput.hlsli"


StructuredBuffer<FrustumCullingOutput>  frustumCullingOutput;
Buffer<uint>                            frustumCullingOutputInstanceCount;

RWStructuredBuffer<ClusterInstanceData> outputClusters;
Buffer<uint>                            outputIndex; // for sharing output buffer

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	if (groupID.x < frustumCullingOutputInstanceCount[0])
	{
		FrustumCullingOutput foutp = frustumCullingOutput[groupID.x];

		uint count = (foutp.clusterCount / 64) + 1;
		for (uint a = 0; a < count; ++a)
		{
			uint clIndex = (a * 64) + groupThreadID.x;
			if (clIndex < foutp.clusterCount)
			{
				ClusterInstanceData idata;
				idata.clusterPointer = foutp.clusterPointer + clIndex;
				idata.instancePointer = foutp.instancePointer;
				idata.uvPointer = foutp.uvPointer;
				idata.lodPointer = foutp.lodPointer;
				outputClusters[outputIndex[0] + foutp.outputPointer + clIndex] = idata;
			}
		}
	}
}
