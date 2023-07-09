#include "engine/rendering/debug/DebugBoundingBoxes.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/rendering/DepthPyramid.h"
#include "components/Camera.h"

namespace engine
{
	#include "shaders/core/shared_types/FrustumCullingOutput.hlsli"
	#include "shaders/core/shared_types/InstanceFrustumCounts.hlsli"
	#include "shaders/core/shared_types/ClusterInstanceData.hlsli"

	DebugBoundingBoxes::DebugBoundingBoxes(Device& device)
		: m_device{ device }
		, m_instanceCollect{ device.createPipeline<shaders::DebugBoundingBoxInstanceCollect>() }
		, m_instanceClusterExpandArgs{ device.createPipeline<shaders::DebugBoundingBoxClusterExpandArgs>() }
		, m_instanceToClusterExpand{ device.createPipeline<shaders::DebugBoundingBoxInstanceToClusterExpand>() }
		, m_debugBoundingBox{ device.createPipeline<shaders::DebugBoundingBox>() }
		, m_createDebugDrawArgs{ device.createPipeline<shaders::DebugBoundingBoxCreateDrawArgs>() }

		, m_instanceCountUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(uint32_t))
				.elements(1)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("ClusterLine cluster count")
			) }
		, m_instanceCountSRV{ device.createBufferSRV(m_instanceCountUAV) }

		, m_allocationUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(uint32_t))
				.elements(1)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("ClusterLine allocation")
			) }
		, m_allocationSingleUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(uint32_t))
				.elements(1)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("ClusterLine allocation")
			) }
		, m_frustumCullingOutputUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(FrustumCullingOutput))
				.elements(MaxInstances)
				.structured(true)
				.usage(ResourceUsage::GpuReadWrite)
				.append(true)
				.name("ClusterLine frustum culling output")
			) }
		, m_frustumCullingOutputSRV{ device.createBufferSRV(m_frustumCullingOutputUAV) }
		, m_clusterCount{ device }
		, m_clusters{ device }
		, m_clusterDraw{ device }

		, m_clusterExpandDispatchArgs{
			device.createBuffer(BufferDescription()
				.format(Format::R32_UINT)
				.elements(3)
				.usage(ResourceUsage::GpuReadWrite)
				.indirectArgument(true)
				.name("ClusterLine index expand dispatch args")
			) }

		, m_clusterExpandDispatchArgsUAV{
			device.createBufferUAV(m_clusterExpandDispatchArgs) }
	{
		m_instanceClusterExpandArgs.cs.expandDispatchArgs = m_clusterExpandDispatchArgsUAV;

		DepthStencilOpDescription front;
		front.StencilFailOp = StencilOp::Keep;
		front.StencilDepthFailOp = StencilOp::Incr;
		front.StencilPassOp = StencilOp::Keep;
		front.StencilFunc = ComparisonFunction::Always;

		DepthStencilOpDescription back;
		back.StencilFailOp = StencilOp::Keep;
		back.StencilDepthFailOp = StencilOp::Decr;
		back.StencilPassOp = StencilOp::Keep;
		back.StencilFunc = ComparisonFunction::Always;

		m_debugBoundingBox.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_debugBoundingBox.setRasterizerState(RasterizerDescription().cullMode(CullMode::None).fillMode(FillMode::Wireframe)); // TODO
		m_debugBoundingBox.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));
	}

	void DebugBoundingBoxes::render(
		CommandList& cmd,
		TextureRTV rtv,
		TextureDSV dsv,
		const Camera& camera,
		Vector2<int> virtualResolution)
	{
		CPU_MARKER(cmd.api(), "Debug bounding boxes");
		GPU_MARKER(cmd, "Debug bounding boxes");

		{
			CPU_MARKER(cmd.api(), "Collect instances");
			GPU_MARKER(cmd, "Collect instances");

			cmd.setStructureCounter(m_frustumCullingOutputUAV, 0);
			cmd.clearBuffer(m_instanceCountUAV);
			cmd.clearBuffer(m_allocationUAV);
			cmd.clearBuffer(m_allocationSingleUAV);
			cmd.clearBuffer(m_clusterCount.uav);

			auto& modelResources = m_device.modelResources();
			engine::vector<Vector4f> frustumPlanes = extractFrustumPlanes(camera.projectionMatrix() * camera.viewMatrix());

			m_instanceCollect.cs.subMeshBoundingBoxes = m_device.modelResources().gpuBuffers().subMeshBoundingBox();
			m_instanceCollect.cs.lodBinding = m_device.modelResources().gpuBuffers().lod();
			m_instanceCollect.cs.instanceLodBinding = m_device.modelResources().gpuBuffers().instanceLodBinding();
			m_instanceCollect.cs.subMeshData = m_device.modelResources().gpuBuffers().subMeshData();
			m_instanceCollect.cs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();

			// constants
			m_instanceCollect.cs.plane0 = frustumPlanes[0];
			m_instanceCollect.cs.plane1 = frustumPlanes[1];
			m_instanceCollect.cs.plane2 = frustumPlanes[2];
			m_instanceCollect.cs.plane3 = frustumPlanes[3];
			m_instanceCollect.cs.plane4 = frustumPlanes[4];
			m_instanceCollect.cs.plane5 = frustumPlanes[5];
			m_instanceCollect.cs.viewMatrix = fromMatrix(camera.viewMatrix());
			m_instanceCollect.cs.projectionMatrix = fromMatrix(camera.projectionMatrix());
			m_instanceCollect.cs.cameraPosition = camera.position();
			m_instanceCollect.cs.instanceCount.x = static_cast<uint32_t>(modelResources.instanceCount());
			m_instanceCollect.cs.size = Vector2f{ static_cast<float>(virtualResolution.x), static_cast<float>(virtualResolution.y) };
			m_instanceCollect.cs.inverseSize = Vector2f{ 1.0f / static_cast<float>(virtualResolution.x), 1.0f / static_cast<float>(virtualResolution.y) };
			m_instanceCollect.cs.pow2size = Vector2f{ static_cast<float>(roundUpToPow2(virtualResolution.x)), static_cast<float>(roundUpToPow2(virtualResolution.y)) };
			m_instanceCollect.cs.farPlaneDistance = camera.farPlane();

			// output
			m_instanceCollect.cs.cullingOutput = m_frustumCullingOutputUAV;
			m_instanceCollect.cs.clusterCountBuffer = m_clusterCount.uav;
			m_instanceCollect.cs.instanceCountBuffer = m_instanceCountUAV;
			m_instanceCollect.cs.outputAllocationShared = m_allocationUAV;
			m_instanceCollect.cs.outputAllocationSharedSingle = m_allocationSingleUAV;

			cmd.bindPipe(m_instanceCollect);
			cmd.dispatch(roundUpToMultiple(modelResources.instanceCount(), 64) / 64, 1, 1);
		}

		{
			CPU_MARKER(cmd.api(), "Create expand instances to clusters args");
			GPU_MARKER(cmd, "Create expand instances to clusters args");

			m_instanceClusterExpandArgs.cs.instanceCountBuffer = m_instanceCountSRV;
			cmd.bindPipe(m_instanceClusterExpandArgs);
			cmd.dispatch(1, 1, 1);
		}

		{
			CPU_MARKER(cmd.api(), "Expand instances to clusters");
			GPU_MARKER(cmd, "Expand instances to clusters");
			
			m_clusters.reset(cmd);

			m_instanceToClusterExpand.cs.frustumCullingOutput = m_frustumCullingOutputSRV;
			m_instanceToClusterExpand.cs.frustumCullingOutputInstanceCount = m_instanceCountSRV;
			m_instanceToClusterExpand.cs.outputClusters = m_clusters.clustersUAV;
			m_instanceToClusterExpand.cs.outputIndex = m_clusters.clusterIndexSRV;

			cmd.bindPipe(m_instanceToClusterExpand);
			cmd.dispatchIndirect(m_clusterExpandDispatchArgs, 0);
		}

		{
			CPU_MARKER(cmd.api(), "Create debug bounding boxes draw arsg");
			GPU_MARKER(cmd, "Create debug bounding boxes draw arsg");
			m_createDebugDrawArgs.cs.clusterSize.x = 36u;
			m_createDebugDrawArgs.cs.inputClusterCount = m_clusterCount.srv;
			m_createDebugDrawArgs.cs.outputDrawCount = m_clusterDraw.clusterRendererExecuteIndirectCount.uav;
			m_createDebugDrawArgs.cs.clusterRendererExecuteIndirectArguments = m_clusterDraw.clusterRendererExecuteIndirectArgumentsBufferUAV;

			cmd.bindPipe(m_createDebugDrawArgs);
			cmd.dispatch(1, 1, 1);
		}

		{
			CPU_MARKER(cmd.api(), "Render cluster debug bounding boxes");
			GPU_MARKER(cmd, "Render cluster debug bounding boxes");

			cmd.setRenderTargets({ rtv }, dsv);

			m_debugBoundingBox.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_debugBoundingBox.vs.normals = m_device.modelResources().gpuBuffers().normal();
			m_debugBoundingBox.vs.tangents = m_device.modelResources().gpuBuffers().tangent();
			m_debugBoundingBox.vs.uv = m_device.modelResources().gpuBuffers().uv();
			m_debugBoundingBox.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_debugBoundingBox.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_debugBoundingBox.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
			m_debugBoundingBox.vs.baseIndexes = m_device.modelResources().gpuBuffers().index();
			m_debugBoundingBox.vs.clusterBoundingBoxes = m_device.modelResources().gpuBuffers().clusterBoundingBox();

			m_debugBoundingBox.vs.clusters = m_clusters.clustersSRV;
			m_debugBoundingBox.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
			m_debugBoundingBox.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());

			cmd.bindPipe(m_debugBoundingBox);
			cmd.executeIndexedIndirect(
				m_device.modelResources().gpuBuffers().indexIBV(),
				m_clusterDraw.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				m_clusterDraw.clusterRendererExecuteIndirectCount.buffer, 0);

		}
	}

}
