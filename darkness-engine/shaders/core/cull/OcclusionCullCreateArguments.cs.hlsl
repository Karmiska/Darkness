
#include "../shared_types/DispatchArgs.hlsli"

Buffer<uint> clusterCountBuffer;
RWStructuredBuffer<DispatchArgs> occlusionDispatchArgs;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint clusterCount = clusterCountBuffer[0];
    uint threadGroupCount = clusterCount / 64;
    if (threadGroupCount * 64 < clusterCount)
        ++threadGroupCount;

    occlusionDispatchArgs[0].threadGroupX = threadGroupCount;
    occlusionDispatchArgs[0].threadGroupY = 1;
    occlusionDispatchArgs[0].threadGroupZ = 1;
}
