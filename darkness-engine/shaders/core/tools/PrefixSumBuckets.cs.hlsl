
#include "BlellochScan.hlsli"
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/FrustumCullingOutput.hlsli"

Buffer<uint> clusterCountBuffer;
StructuredBuffer<FrustumCullingOutput> frustumCullingOutput;
RWBuffer<uint> output;

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
    uint tid = dispatchThreadID.x + 1;
    uint x = frustumCullingOutput[dispatchThreadID.x].clusterCount;
    uint o = scan(dispatchThreadID, groupThreadID.x, x);
    if(tid < clusterCountBuffer[1])
        output[tid] = o;
}
