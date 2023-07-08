
#include "../shared_types/DispatchArgs.hlsli"

Buffer<uint> clusterCountBuffer;
RWStructuredBuffer<DispatchArgs> expandDispatchArgs;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint indexCount = clusterCountBuffer[0] * 192;
    uint iThreadGroupCount = indexCount / 64;
    if (iThreadGroupCount * 64 < indexCount)
        ++iThreadGroupCount;

    expandDispatchArgs[0].threadGroupX = iThreadGroupCount;
    expandDispatchArgs[0].threadGroupY = 1;
    expandDispatchArgs[0].threadGroupZ = 1;
}
