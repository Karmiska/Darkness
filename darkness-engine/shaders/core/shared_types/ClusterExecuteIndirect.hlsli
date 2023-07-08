#ifdef __cplusplus
#pragma once
#endif

struct ClusterExecuteIndirectArgs
{
    uint clusterStart;
	uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    uint StartInstanceLocation;
};
