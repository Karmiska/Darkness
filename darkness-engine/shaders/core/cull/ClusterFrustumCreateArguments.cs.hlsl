
#include "../shared_types/DispatchArgs.hlsli"

Buffer<uint> clusterCountBuffer;
RWStructuredBuffer<DispatchArgs> clusterFrustumDispatchArgs;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint clusterCount = clusterCountBuffer[0];
    uint iThreadGroupCount = clusterCount / 64;
    if (iThreadGroupCount * 64 < clusterCount)
        ++iThreadGroupCount;

    clusterFrustumDispatchArgs[0].threadGroupX = iThreadGroupCount;
    clusterFrustumDispatchArgs[0].threadGroupY = 1;
    clusterFrustumDispatchArgs[0].threadGroupZ = 1;
}
