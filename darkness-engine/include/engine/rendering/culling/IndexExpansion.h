#pragma once

#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"
#include "shaders/core/cull/ClustersToIndexExpand.h"
#include "shaders/core/cull/ClustersToIndexExpandCreateArguments.h"
#include "engine/primitives/Vector2.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/tools/BufferMath.h"
#include "engine/rendering/GpuBuffers.h"

#include "containers/memory.h"

namespace engine
{
    class CommandList;
    class Camera;
    class ModelResources;
    class ClusterExpansion;
    class DepthPyramid;

    class IndexExpansion
    {
    public:
        IndexExpansion(Device& device);

        void expandIndexes(
            Device& device,
            CommandList& cmd,
            ClusterDataLine& input,
            IndexDataLine& indexAppendOutput,
            IndexDataLine& indexDrawOutput,
            DrawDataLine& drawOutput,
			uint32_t clusterSize = 192);

    private:
		GpuBuffers& m_gpuBuffers;
        engine::Pipeline<shaders::ClustersToIndexExpand> m_indexExpand;
        engine::Pipeline<shaders::ClustersToIndexExpandCreateArguments> m_createArguments;

        BufferOwner      m_expandDispatchArgs;
        BufferUAVOwner   m_expandDispatchArgsUAV;

        BufferMath  m_bufferMath;
    };
}

