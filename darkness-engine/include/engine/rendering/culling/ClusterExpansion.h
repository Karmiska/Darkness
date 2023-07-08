#pragma once

#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"
#include "shaders/core/cull/InstanceToClusterExpand.h"
#include "shaders/core/cull/InstanceToClusterExpandCreateArguments.h"
#include "engine/rendering/culling/ModelDataLine.h"

namespace engine
{
    class Device;
    class CommandList;

    class ClusterExpansion
    {
    public:
        ClusterExpansion(Device& device);

        void expandClusters(
            CommandList& cmd,
            BufferSRV frustumCullingOutput,
            BufferSRV frustumCullingOutputInstanceCount,
            ClusterDataLine& clusters);
    private:
        engine::Pipeline<shaders::InstanceToClusterExpand> m_clusterExpand;
        engine::Pipeline<shaders::InstanceToClusterExpandCreateArguments> m_instanceToClusterExpandCreateArguments;

        BufferOwner      m_clusterExpandDispatchArgs;
        BufferUAVOwner   m_clusterExpandDispatchArgsUAV;
    };
}
