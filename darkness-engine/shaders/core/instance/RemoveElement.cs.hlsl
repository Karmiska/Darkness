
#include "../shared_types/TransformHistory.hlsli"
#include "../shared_types/LodBinding.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"
#include "../shared_types/VertexScale.hlsli"

RWStructuredBuffer<TransformHistory> transform;
RWStructuredBuffer<LodBinding> lodBinding;
RWStructuredBuffer<InstanceMaterial> materials;
RWStructuredBuffer<VertexScale> scales;
RWBuffer<uint> clusterTracking;

cbuffer CopyConstants
{
    uint elementToRemove;
    uint lastElement;
    uint2 padding;
};

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    transform[elementToRemove] = transform[lastElement];
    lodBinding[elementToRemove] = lodBinding[lastElement];
    materials[elementToRemove] = materials[lastElement];
    scales[elementToRemove] = scales[lastElement];
    clusterTracking[elementToRemove] = clusterTracking[lastElement];
}
