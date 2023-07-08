#ifdef __cplusplus
#pragma once
#endif

struct SubMeshAdjacency
{
    uint adjacencyPointer;
    uint adjacencyCount;
    uint baseVertexPointer;

    uint padding;
};
