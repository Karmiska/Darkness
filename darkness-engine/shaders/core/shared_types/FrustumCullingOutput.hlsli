#ifdef __cplusplus
#pragma once
#endif

struct FrustumCullingOutput
{
    uint clusterPointer;
    uint clusterCount;
    uint instancePointer;
    uint outputPointer;
    uint uvPointer;
    uint lodPointer;

    uint padding1;
    uint padding2;
};
