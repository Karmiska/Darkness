#include "../shared_types/ClusterInstanceData.hlsli"
#include "../shared_types/ClusterData.hlsli"
#include "../shared_types/ClusterExecuteIndirect.hlsli"

StructuredBuffer<ClusterInstanceData>   inputClusters;
Buffer<uint>                            inputClusterCount;
Buffer<uint>                            inputClustersIndex;

RWBuffer<uint>                          outputIndexes;
RWBuffer<uint>                          outputIndexCount;
Buffer<uint>                            outputIndexIndex;

// create also the needed draw arguments for the index buffer
RWStructuredBuffer<ClusterExecuteIndirectArgs>  clusterRendererExecuteIndirectArguments;
RWBuffer<uint>                                  clusterRendererExecuteIndirectCountBuffer;

StructuredBuffer<ClusterData>   clusterData;
Buffer<uint>                    baseIndexes;

cbuffer CullingConstants
{
    // 16
    uint hack;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
    float maxClusters = inputClusterCount[0];
    uint threads = maxClusters * 192;

    if (dispatchThreadID.x < threads)
    {
        uint inputClusterIndex = inputClustersIndex[0];
        uint clusterId = dispatchThreadID.x / 192;
        uint clusterIndexId = dispatchThreadID.x - (clusterId * 192);
        ClusterInstanceData cInstanceData = inputClusters[inputClusterIndex + clusterId];
        ClusterData cdata = clusterData[cInstanceData.clusterPointer];

        if (clusterIndexId < cdata.indexCount)
        {
            uint index = baseIndexes[cdata.indexPointer + clusterIndexId];
            uint truncatedClusterId = clusterId / 65536;
            truncatedClusterId = clusterId - (truncatedClusterId * 65536);
            outputIndexes[outputIndexIndex[0] + dispatchThreadID.x] = ((truncatedClusterId & 0x0000ffff) << 16) | (index & 0x0000ffff);
        }
        else
            outputIndexes[outputIndexIndex[0] + dispatchThreadID.x] = 0xffffffff;

        // 65536 clusters with 192 indexes each, means
        // 12582912 indexes fit in one draw call
        // and we can keep 16 bit cluster index
        uint currentDrawCall = dispatchThreadID.x / hack;// 12582912;
        uint threadId = dispatchThreadID.x;
        uint temp = currentDrawCall * hack;// 12582912;
        if (temp == threadId)
        {
            uint maxDrawCalls = ceil(maxClusters / 65536.0f);
            uint clusterCount;
            if (currentDrawCall == maxDrawCalls - 1)
            {
                clusterCount = maxClusters - (currentDrawCall * 65536);
            }
            else
            {
                clusterCount = 65536;
            }

            clusterRendererExecuteIndirectArguments[currentDrawCall].clusterStart = inputClusterIndex + clusterId;
            clusterRendererExecuteIndirectArguments[currentDrawCall].IndexCountPerInstance = clusterCount * 192;
            clusterRendererExecuteIndirectArguments[currentDrawCall].InstanceCount = 1;
            clusterRendererExecuteIndirectArguments[currentDrawCall].StartIndexLocation = outputIndexIndex[0] + dispatchThreadID.x;
            clusterRendererExecuteIndirectArguments[currentDrawCall].BaseVertexLocation = 0;
            clusterRendererExecuteIndirectArguments[currentDrawCall].StartInstanceLocation = 0;

            InterlockedAdd(clusterRendererExecuteIndirectCountBuffer[0], 1);
        }

        if (dispatchThreadID.x == 0)
        {
            outputIndexCount[0] = outputIndexCount[0] + threads;
        }
    }
}
