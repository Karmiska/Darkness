
Buffer<uint> allocations;

struct DrawIndexIndirectArgs
{
	uint IndexCountPerInstance;
	uint InstanceCount;
	uint StartIndexLocation;
	int BaseVertexLocation;
	uint StartInstanceLocation;
};
RWStructuredBuffer<DrawIndexIndirectArgs> debugDrawArgs;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint count = allocations[0];
	uint indexCount = count * 36;
	uint vertexCount = count * 24;

	debugDrawArgs[0].IndexCountPerInstance = indexCount;
	debugDrawArgs[0].InstanceCount = 1;
	debugDrawArgs[0].StartIndexLocation = 0;
	debugDrawArgs[0].BaseVertexLocation = 0;
	debugDrawArgs[0].StartInstanceLocation = 0;
}
