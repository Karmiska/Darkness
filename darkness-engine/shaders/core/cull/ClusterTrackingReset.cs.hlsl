
RWBuffer<uint> clusterTracking;
Buffer<uint> clusterTrackingResetIndexes;
Buffer<uint> clusterTrackingCount;

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < clusterTrackingCount[0])
    {
        clusterTracking[clusterTrackingResetIndexes[dispatchThreadID.x]] = 0xffffffff;
    }
}
