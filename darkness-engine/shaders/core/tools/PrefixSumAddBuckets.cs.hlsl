
#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/FrustumCullingOutput.hlsli"

Buffer<uint> clusterCountBuffer;
Buffer<uint> input1;
Buffer<uint> input2;
RWBuffer<uint> output;

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
    if (dispatchThreadID.x < clusterCountBuffer[1])
    {
        output[dispatchThreadID.x+1] = input1[dispatchThreadID.x+1] + input2[dispatchThreadID.x+1];
    }
}
