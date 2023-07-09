
#include "../shared_types/DispatchArgs.hlsli"

Buffer<uint> instanceCountBuffer;
RWStructuredBuffer<DispatchArgs> expandDispatchArgs;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint instanceCount = instanceCountBuffer[0] * 64;
	uint iThreadGroupCount = instanceCount / 64;
	if (iThreadGroupCount * 64 < instanceCount)
		++iThreadGroupCount;

	expandDispatchArgs[0].threadGroupX = iThreadGroupCount;
	expandDispatchArgs[0].threadGroupY = 1;
	expandDispatchArgs[0].threadGroupZ = 1;
}
