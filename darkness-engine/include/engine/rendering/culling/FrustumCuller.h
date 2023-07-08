#pragma once

#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"
#include "shaders/core/cull/InstanceFrustum.h"
#include "shaders/core/cull/InstanceFrustumNoDepth.h"
#include "shaders/core/cull/InstanceShadowFrustum.h"
#include "shaders/core/cull/ClusterFrustum.h"
#include "shaders/core/cull/ClusterFrustumCreateArguments.h"
#include "engine/primitives/Vector2.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/tools/BufferMath.h"
#include "engine/graphics/Sampler.h"
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

    class FrustumCuller
    {
    public:
        FrustumCuller(Device& device);

        void instanceCull(
            CommandList& cmd,
            Camera& camera,
            ModelResources& modelResources,
            Vector2<int> virtualResolution,
            DepthPyramid* depthPyramid,
            ClusterDataLine& appendOutput,
            ClusterDataLine& drawOutput);

        void instanceCullNoDepth(
            CommandList& cmd,
            const Camera& camera,
            ModelResources& modelResources,
            Vector2<int> virtualResolution,
            ClusterDataLine& appendOutput,
            ClusterDataLine& drawOutput);

        void instanceShadowCull(
            CommandList& cmd,
            Camera& camera,
            ModelResources& modelResources,
            Vector2<int> virtualResolution,
            ClusterDataLine& output,
            ClusterDataLine& outputAlphaClipped,
			ClusterDataLine& outputTerrain,
            BufferSRV sceneChanges,
            BufferSRV changeCount,
            BufferUAV matchCount,
            bool forceShadowPass);

        void clusterCull(
            CommandList& cmd,
            Camera& camera,
            ModelResources& modelResources,
            Vector2<int> virtualResolution,
            ClusterDataLine& input,

            ClusterDataLine& appendOutput,
            ClusterDataLine& appendAlphaclippedOutput,
            ClusterDataLine& appendTransparentOutput,
			ClusterDataLine& appendTerrainOutput,

            ClusterDataLine& drawOutput,
            ClusterDataLine& drawAlphaclippedOutput,
            ClusterDataLine& drawTransparentOutput,
			ClusterDataLine& drawTerrainOutput,

            BufferSRV clusterTrackingInstanceToCluster,
            BufferUAV clusterTrackingClusters);

        void expandClusters(
            CommandList& cmd, 
            ClusterDataLine& output);

        void expandClustersShadowAlphaClip(
            CommandList& cmd,
            ClusterDataLine& output);

		void expandClustersShadowTerrain(
			CommandList& cmd,
			ClusterDataLine& output);

        void clearInstanceCount(CommandList& cmd);

        BufferSRV instanceCount()
        {
            return m_instanceCountSRV;
        }
    private:
        Device& m_device;
		GpuBuffers& m_gpuBuffers;
        engine::Pipeline<shaders::InstanceFrustum> m_instanceFrustum;
        engine::Pipeline<shaders::InstanceFrustumNoDepth> m_instanceFrustumNoDepth;
        engine::Pipeline<shaders::InstanceShadowFrustum> m_instanceShadowFrustum;
        engine::Pipeline<shaders::ClusterFrustum> m_clusterFrustum;
        engine::Pipeline<shaders::ClusterFrustumCreateArguments> m_clusterFrustumCreateArguments;
        engine::shared_ptr<ClusterExpansion> m_clusterExpansion;

        BufferUAVOwner   m_instanceCountUAV;
        BufferSRVOwner   m_instanceCountSRV;

        BufferUAVOwner   m_allocationUAV;
        BufferUAVOwner   m_allocationSingleUAV;
        BufferUAVOwner   m_frustumCullingOutputUAV;
        BufferSRVOwner   m_frustumCullingOutputSRV;

        BufferOwner      m_clusterCullDispatchArgs;
        BufferUAVOwner   m_clusterCullDispatchArgsUAV;

        Sampler     m_depthSampler;

        // TODO: having single instance reused
        // causes gpu validation errors in CBV state.
        // find out why.
        BufferMath  m_bufferMath;
        BufferMath  m_bufferMathSecond;

        BufferUAVOwner   m_instanceCountAlphaClipUAV;
        BufferSRVOwner   m_instanceCountAlphaClipSRV;

        BufferUAVOwner   m_allocationAlphaClipUAV;
        BufferUAVOwner   m_allocationAlphaClipSingleUAV;
        BufferUAVOwner   m_frustumCullingOutputAlphaClipUAV;
        BufferSRVOwner   m_frustumCullingOutputAlphaClipSRV;

		BufferUAVOwner   m_instanceCountTerrainUAV;
		BufferSRVOwner   m_instanceCountTerrainSRV;

		BufferUAVOwner   m_allocationTerrainUAV;
        BufferUAVOwner   m_allocationTerrainSingleUAV;
		BufferUAVOwner   m_frustumCullingOutputTerrainUAV;
		BufferSRVOwner   m_frustumCullingOutputTerrainSRV;
    };
}
