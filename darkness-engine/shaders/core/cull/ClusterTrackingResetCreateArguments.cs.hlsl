
#include "../shared_types/DispatchArgs.hlsli"

Buffer<uint> clusterTrackingCount;
RWStructuredBuffer<DispatchArgs> clusterTrackingResetDispatchArgs;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint trackingCount = clusterTrackingCount[0];
    uint iThreadGroupCount = trackingCount / 64;
    if (iThreadGroupCount * 64 < trackingCount)
        ++iThreadGroupCount;

    clusterTrackingResetDispatchArgs[0].threadGroupX = iThreadGroupCount;
    clusterTrackingResetDispatchArgs[0].threadGroupY = 1;
    clusterTrackingResetDispatchArgs[0].threadGroupZ = 1;
}
