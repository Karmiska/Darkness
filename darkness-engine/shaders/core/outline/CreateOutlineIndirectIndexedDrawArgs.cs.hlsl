
#include "../shared_types/SubMeshAdjacency.hlsli"
#include "../shared_types/LodBinding.hlsli"

Buffer<uint> lodBinding;
StructuredBuffer<LodBinding> instanceLodBinding;
StructuredBuffer<SubMeshAdjacency> subMeshAdjacency;
RWBuffer<uint> indexedDrawArguments;

cbuffer OutlineConstants
{
    uint instanceId;
};

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    LodBinding lodBind = instanceLodBinding[instanceId];
    uint subMeshId = lodBinding[lodBind.lodPointer];
    SubMeshAdjacency subMeshAdj = subMeshAdjacency[subMeshId];

    indexedDrawArguments[0] = subMeshAdj.adjacencyCount;
    indexedDrawArguments[1] = 1;
    indexedDrawArguments[2] = subMeshAdj.adjacencyPointer;
    indexedDrawArguments[3] = subMeshAdj.baseVertexPointer;
    indexedDrawArguments[4] = 0;
}
