
Buffer<uint> voxelListCount;

struct DispatchArgs
{
	uint threadGroupX;
	uint threadGroupY;
	uint threadGroupZ;
	uint padding;
};
RWStructuredBuffer<DispatchArgs> voxelListDispatchArgs;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	voxelListDispatchArgs[0].threadGroupX = (voxelListCount[0]+63) / 64;
	voxelListDispatchArgs[0].threadGroupY = 1;
	voxelListDispatchArgs[0].threadGroupZ = 1;
}
