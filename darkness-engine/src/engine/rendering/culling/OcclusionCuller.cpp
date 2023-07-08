#include "engine/rendering/culling/OcclusionCuller.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/rendering/ModelResources.h"
#include "engine/rendering/BufferSettings.h"
#include "engine/rendering/culling/ClusterExpansion.h"
#include "engine/rendering/DepthPyramid.h"
#include "components/Camera.h"

namespace engine
{
    #include "shaders/core/shared_types/ClusterInstanceData.hlsli"

    OcclusionCuller::OcclusionCuller(Device& device)
        : m_device{ device }
        , m_gpuBuffers{ device.modelResources().gpuBuffers() }
		, m_occlusionCull{ device.createPipeline<shaders::OcclusionCull>() }
        , m_occlusionCullCreateArguments{ device.createPipeline<shaders::OcclusionCullCreateArguments>() }

        , m_depthSampler{ device.createSampler(SamplerDescription()
            .filter(Filter::Point)
            .textureAddressMode(TextureAddressMode::Clamp)) }

        , m_occlusionCullingDispatchArgs{
            device.createBuffer(BufferDescription()
                .elementSize(sizeof(DispatchIndirectArgs))
                .elements(1)
                .usage(ResourceUsage::GpuReadWrite)
                .structured(true)
				.indirectArgument(true)
                .name("ClusterLine Occlusion culling dispatch args")
            ) }
        , m_occlusionCullingDispatchArgsUAV{ device.createBufferUAV(m_occlusionCullingDispatchArgs) }

        , m_bufferMath{ device }
    {
        m_occlusionCull.cs.depthSampler = m_depthSampler;
        m_occlusionCull.cs.emitAll = false;

        m_occlusionCullCreateArguments.cs.occlusionDispatchArgs = m_occlusionCullingDispatchArgsUAV;
    }

    void OcclusionCuller::occlusionCull(
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
            BufferUAV clusterTrackingClusters)
    {
        {
            CPU_MARKER(cmd.api(), "Create occlusion culling arguments");
            GPU_MARKER(cmd, "Create occlusion culling arguments");
            m_occlusionCullCreateArguments.cs.clusterCountBuffer = input.clusterCount.srv;
            
            cmd.bindPipe(m_occlusionCullCreateArguments);
            cmd.dispatch(1, 1, 1);
        }
        {
            CPU_MARKER(cmd.api(), "Occlusion culling");
            GPU_MARKER(cmd, "Occlusion culling");

            drawOutputAll.clustersSRV = appendOutputAll.clustersSRV;
            drawOutputAll.clustersUAV = appendOutputAll.clustersUAV;
            cmd.copyBuffer(appendOutputAll.clusterCount.buffer, drawOutputAll.clusterIndexUAV.resource().buffer(), 1);

            drawOutputNotYetDrawnDepth.clustersSRV = appendOutputNotYetDrawnDepth.clustersSRV;
            drawOutputNotYetDrawnDepth.clustersUAV = appendOutputNotYetDrawnDepth.clustersUAV;
            cmd.copyBuffer(appendOutputNotYetDrawnDepth.clusterCount.buffer, drawOutputNotYetDrawnDepth.clusterIndexUAV.resource().buffer(), 1);

            drawOutputAlphaClipped.clustersSRV = appendOutputAlphaClipped.clustersSRV;
            drawOutputAlphaClipped.clustersUAV = appendOutputAlphaClipped.clustersUAV;
            cmd.copyBuffer(appendOutputAlphaClipped.clusterCount.buffer, drawOutputAlphaClipped.clusterIndexUAV.resource().buffer(), 1);

            drawOutputTransparent.clustersSRV = appendOutputTransparent.clustersSRV;
            drawOutputTransparent.clustersUAV = appendOutputTransparent.clustersUAV;
            cmd.copyBuffer(appendOutputTransparent.clusterCount.buffer, drawOutputTransparent.clusterIndexUAV.resource().buffer(), 1);

			drawOutputTerrain.clustersSRV = appendOutputTerrain.clustersSRV;
			drawOutputTerrain.clustersUAV = appendOutputTerrain.clustersUAV;
			cmd.copyBuffer(appendOutputTerrain.clusterCount.buffer, drawOutputTerrain.clusterIndexUAV.resource().buffer(), 1);

			m_occlusionCull.cs.boundingBoxes = m_gpuBuffers.clusterBoundingBox();
			m_occlusionCull.cs.transformHistory = m_gpuBuffers.instanceTransform();
			m_occlusionCull.cs.clusterCones = m_gpuBuffers.clusterCone();
			m_occlusionCull.cs.instanceMaterials = m_gpuBuffers.instanceMaterial();

            // constants
            {
                engine::vector<Vector4f> frustumPlanes = extractFrustumPlanes(camera.projectionMatrix() * camera.viewMatrix());
                m_occlusionCull.cs.viewMatrix = fromMatrix(camera.viewMatrix());
                m_occlusionCull.cs.invViewMatrix = fromMatrix(camera.viewMatrix().inverse());
                m_occlusionCull.cs.projectionMatrix = fromMatrix(camera.projectionMatrix());
                m_occlusionCull.cs.cameraPosition = Vector4f(camera.position(), 1.0f);
                m_occlusionCull.cs.size = Vector2f{ static_cast<float>(virtualResolution.x), static_cast<float>(virtualResolution.y) };
                m_occlusionCull.cs.inverseSize = Vector2f{ 1.0f / static_cast<float>(virtualResolution.x), 1.0f / static_cast<float>(virtualResolution.y) };
                m_occlusionCull.cs.pow2size = Vector2f{ static_cast<float>(roundUpToPow2(virtualResolution.x)), static_cast<float>(roundUpToPow2(virtualResolution.y)) };
                m_occlusionCull.cs.nearFar = Vector2f(camera.nearPlane(), camera.farPlane());
                m_occlusionCull.cs.mipLevels.x = static_cast<float>(depthPyramid ? depthPyramid->depth().texture().mipLevels() : 1);
                m_occlusionCull.cs.mipLevels.y = m_occlusionCull.cs.mipLevels.x;
                m_occlusionCull.cs.mipLevels.z = m_occlusionCull.cs.mipLevels.x;
                m_occlusionCull.cs.mipLevels.w = m_occlusionCull.cs.mipLevels.x;
                m_occlusionCull.cs.plane0 = frustumPlanes[0];
                m_occlusionCull.cs.plane1 = frustumPlanes[1];
                m_occlusionCull.cs.plane2 = frustumPlanes[2];
                m_occlusionCull.cs.plane3 = frustumPlanes[3];
                m_occlusionCull.cs.plane4 = frustumPlanes[4];
                m_occlusionCull.cs.plane5 = frustumPlanes[5];
                m_occlusionCull.cs.cameraDirection = camera.forward();
                m_occlusionCull.cs.fnvPrime.x = 16777619u;
                m_occlusionCull.cs.fnvOffsetBasis.x = 2166136261u;
                m_occlusionCull.cs.farPlaneDistance = camera.farPlane();
            }

            // inputs
            {
                m_occlusionCull.cs.inputClusterCount = input.clusterCount.srv;
                m_occlusionCull.cs.inputClusterIndex = input.clusterIndexSRV;
                m_occlusionCull.cs.clusterAccurateTrackingInstancePtr = clusterTrackingInstanceToCluster;
                m_occlusionCull.cs.clusterAccurateTracking = clusterTrackingClusters;
                m_occlusionCull.cs.lodBinding = m_device.modelResources().gpuBuffers().lod();
                m_occlusionCull.cs.instanceLodBinding = m_device.modelResources().gpuBuffers().instanceLodBinding();
                m_occlusionCull.cs.subMeshData = m_device.modelResources().gpuBuffers().subMeshData();
            }

            // outputs
            {
                m_occlusionCull.cs.outputClusters = appendOutputAll.clustersUAV;
                m_occlusionCull.cs.outputClusterDistributor = appendOutputAll.clusterCount.uav;
                m_occlusionCull.cs.outputClusterIndex = appendOutputAll.clusterIndexSRV;

                // second GBuffer pass
                m_occlusionCull.cs.notDrawnOutputClusters = appendOutputNotYetDrawnDepth.clustersUAV;
                m_occlusionCull.cs.notDrawnOutputClusterDistributor = appendOutputNotYetDrawnDepth.clusterCount.uav;

                // alphaclipped clusters
                m_occlusionCull.cs.alphaClippedOutputClusters = appendOutputAlphaClipped.clustersUAV;
                m_occlusionCull.cs.alphaClippedOutputClusterDistributor = appendOutputAlphaClipped.clusterCount.uav;
                m_occlusionCull.cs.alphaClippedOutputClusterIndex = appendOutputAlphaClipped.clusterIndexSRV;

                // alphaclipped clusters
                m_occlusionCull.cs.transparentOutputClusters = appendOutputTransparent.clustersUAV;
                m_occlusionCull.cs.transparentOutputClusterDistributor = appendOutputTransparent.clusterCount.uav;
                m_occlusionCull.cs.transparentOutputClusterIndex = appendOutputTransparent.clusterIndexSRV;

				// terrain clusters
				m_occlusionCull.cs.terrainOutputClusters = appendOutputTerrain.clustersUAV;
				m_occlusionCull.cs.terrainOutputClusterDistributor = appendOutputTerrain.clusterCount.uav;
				m_occlusionCull.cs.terrainOutputClusterIndex = appendOutputTerrain.clusterIndexSRV;
            }

            // buffers
            m_occlusionCull.cs.depthPyramid = depthPyramid != nullptr ? depthPyramid->depth() : TextureSRV();

            cmd.bindPipe(m_occlusionCull);
            cmd.dispatchIndirect(m_occlusionCullingDispatchArgs, 0);

            m_bufferMath.perform(
                cmd,
                appendOutputAll.clusterCount.srv,
                drawOutputAll.clusterIndexSRV,
                drawOutputAll.clusterCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

            m_bufferMath.perform(
                cmd,
                appendOutputNotYetDrawnDepth.clusterCount.srv,
                drawOutputNotYetDrawnDepth.clusterIndexSRV,
                drawOutputNotYetDrawnDepth.clusterCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

            m_bufferMath.perform(
                cmd,
                appendOutputAlphaClipped.clusterCount.srv,
                drawOutputAlphaClipped.clusterIndexSRV,
                drawOutputAlphaClipped.clusterCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

            m_bufferMath.perform(
                cmd,
                appendOutputTransparent.clusterCount.srv,
                drawOutputTransparent.clusterIndexSRV,
                drawOutputTransparent.clusterCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

			m_bufferMath.perform(
				cmd,
				appendOutputTerrain.clusterCount.srv,
				drawOutputTerrain.clusterIndexSRV,
				drawOutputTerrain.clusterCount.uav,
				BufferMathOperation::Subtraction, 1, 0, 0, 0);
        }
    }
}
