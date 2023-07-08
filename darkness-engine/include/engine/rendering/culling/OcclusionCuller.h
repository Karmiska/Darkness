#pragma once

#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/graphics/Sampler.h"
#include "shaders/core/cull/OcclusionCull.h"
#include "shaders/core/cull/OcclusionCullCreateArguments.h"
#include "engine/primitives/Vector2.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/tools/BufferMath.h"
#include "engine/rendering/GpuBuffers.h"

#include "containers/memory.h"

namespace engine
{
    class Device;
    class CommandList;
    class Camera;
    class ModelResources;
    class ClusterExpansion;
    class DepthPyramid;

    class OcclusionCuller
    {
    public:
        OcclusionCuller(Device& device);

        void occlusionCull(
            CommandList& cmd,
            Camera& camera,
            Vector2<int> virtualResolution,
            DepthPyramid* depthPyramid,

            ClusterDataLine& input,

            ClusterDataLine& appendOutputAll,
            ClusterDataLine& appendOutputNotYetDrawnDepth,
            ClusterDataLine& appendOutputAlphaClipped,
            ClusterDataLine& appendOutputTransparent,
			ClusterDataLine& appendOutputTerrain,

            ClusterDataLine& drawOutputAll,
            ClusterDataLine& drawOutputNotYetDrawnDepth,
            ClusterDataLine& drawOutputAlphaClipped,
            ClusterDataLine& drawOutputTransparent,
			ClusterDataLine& drawOutputTerrain,

            BufferSRV clusterTrackingInstanceToCluster,
            BufferUAV clusterTrackingClusters);

    private:
        Device& m_device;
		GpuBuffers& m_gpuBuffers;
        engine::Pipeline<shaders::OcclusionCull> m_occlusionCull;
        engine::Pipeline<shaders::OcclusionCullCreateArguments> m_occlusionCullCreateArguments;

        Sampler     m_depthSampler;

        BufferOwner      m_occlusionCullingDispatchArgs;
        BufferUAVOwner   m_occlusionCullingDispatchArgsUAV;

        BufferMath  m_bufferMath;
    };
}
