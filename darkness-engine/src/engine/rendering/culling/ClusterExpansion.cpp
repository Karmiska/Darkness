#include "engine/rendering/culling/ClusterExpansion.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    #include "shaders/core/shared_types/DispatchArgs.hlsli"

    ClusterExpansion::ClusterExpansion(Device& device)
        : m_clusterExpand{ device.createPipeline<shaders::InstanceToClusterExpand>() }
        , m_instanceToClusterExpandCreateArguments{ device.createPipeline<shaders::InstanceToClusterExpandCreateArguments>() }

        , m_clusterExpandDispatchArgs{
            device.createBuffer(BufferDescription()
                .structured(true)
                .elementSize(sizeof(DispatchArgs))
                .elements(1)
                .usage(ResourceUsage::GpuReadWrite)
				.indirectArgument(true)
                .name("ClusterLine index expand dispatch args")
            ) }

        , m_clusterExpandDispatchArgsUAV{
            device.createBufferUAV(m_clusterExpandDispatchArgs) }
    {
        m_instanceToClusterExpandCreateArguments.cs.expandDispatchArgs = m_clusterExpandDispatchArgsUAV;
    }

    void ClusterExpansion::expandClusters(
        CommandList& cmd,
        BufferSRV frustumCullingOutput,
        BufferSRV frustumCullingOutputInstanceCount,
        ClusterDataLine& clusters)
    {
        {
            CPU_MARKER(cmd.api(), "Create instance to cluster expansion arguments");
            GPU_MARKER(cmd, "Create instance to cluster expansion arguments");

            m_instanceToClusterExpandCreateArguments.cs.instanceCountBuffer = frustumCullingOutputInstanceCount;
            cmd.bindPipe(m_instanceToClusterExpandCreateArguments);
            cmd.dispatch(1, 1, 1);
        }

        {
            if (clusters.clustersUAV.resource().valid())
            {
                CPU_MARKER(cmd.api(), "Expand instance clusters");
                GPU_MARKER(cmd, "Expand instance clusters");

                // input
                m_clusterExpand.cs.frustumCullingOutput = frustumCullingOutput;
                m_clusterExpand.cs.frustumCullingOutputInstanceCount = frustumCullingOutputInstanceCount;

                m_clusterExpand.cs.outputClusters = clusters.clustersUAV;
                m_clusterExpand.cs.outputIndex = clusters.clusterIndexSRV;

                cmd.bindPipe(m_clusterExpand);
                cmd.dispatchIndirect(m_clusterExpandDispatchArgs, 0);
            }
        }
    }
}