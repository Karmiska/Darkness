#include "engine/rendering/culling/FrustumCuller.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/rendering/ModelResources.h"
#include "engine/rendering/BufferSettings.h"
#include "engine/rendering/culling/ClusterExpansion.h"
#include "engine/rendering/DepthPyramid.h"
#include "components/Camera.h"

namespace engine
{
    #include "shaders/core/shared_types/FrustumCullingOutput.hlsli"
    #include "shaders/core/shared_types/InstanceFrustumCounts.hlsli"
    #include "shaders/core/shared_types/ClusterInstanceData.hlsli"

    FrustumCuller::FrustumCuller(Device& device)
        : m_device{ device }
        , m_gpuBuffers{ device.modelResources().gpuBuffers() }
		, m_instanceFrustum{ device.createPipeline<shaders::InstanceFrustum>() }
        , m_instanceFrustumNoDepth{ device.createPipeline<shaders::InstanceFrustumNoDepth>() }
        , m_instanceShadowFrustum{ device.createPipeline<shaders::InstanceShadowFrustum>() }
        , m_clusterFrustum{ device.createPipeline<shaders::ClusterFrustum>() }
        , m_clusterFrustumCreateArguments{ device.createPipeline<shaders::ClusterFrustumCreateArguments>() }
        , m_clusterExpansion{ engine::make_shared<ClusterExpansion>(device) }

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
                .name("ClusterLine single allocation")
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

        , m_clusterCullDispatchArgs{
            device.createBuffer(BufferDescription()
                .elementSize(sizeof(DispatchIndirectArgs))
                .elements(1)
                .usage(ResourceUsage::GpuReadWrite)
                .structured(true)
				.indirectArgument(true)
                .name("ClusterLine Occlusion culling dispatch args")
            ) }
        , m_clusterCullDispatchArgsUAV{ device.createBufferUAV(m_clusterCullDispatchArgs) }

        , m_depthSampler{ device.createSampler(SamplerDescription()
            .filter(Filter::Point)
            .textureAddressMode(TextureAddressMode::Clamp)) }
        
        , m_bufferMath{ device }
        , m_bufferMathSecond{ device }

        , m_instanceCountAlphaClipUAV{
            device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(1)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("ClusterLine cluster count")
            ) }
        , m_instanceCountAlphaClipSRV{ device.createBufferSRV(m_instanceCountAlphaClipUAV) }

        , m_allocationAlphaClipUAV{
            device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(1)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("ClusterLine allocation")
            ) }
        , m_allocationAlphaClipSingleUAV{
            device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(1)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("ClusterLine allocation")
            ) }
        , m_frustumCullingOutputAlphaClipUAV{
            device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(FrustumCullingOutput))
                .elements(MaxInstances)
                .structured(true)
                .usage(ResourceUsage::GpuReadWrite)
                .append(true)
                .name("ClusterLine frustum culling output")
            ) }
        , m_frustumCullingOutputAlphaClipSRV{ device.createBufferSRV(m_frustumCullingOutputAlphaClipUAV) }

		, m_instanceCountTerrainUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(uint32_t))
				.elements(1)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("ClusterLine cluster count")
			) }
		, m_instanceCountTerrainSRV{ device.createBufferSRV(m_instanceCountTerrainUAV) }

		, m_allocationTerrainUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(uint32_t))
				.elements(1)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("ClusterLine allocation")
			) }
        , m_allocationTerrainSingleUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(uint32_t))
				.elements(1)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("ClusterLine allocation")
			) }
		, m_frustumCullingOutputTerrainUAV{
			device.createBufferUAV(BufferDescription()
				.elementSize(sizeof(FrustumCullingOutput))
				.elements(MaxInstances)
				.structured(true)
				.usage(ResourceUsage::GpuReadWrite)
				.append(true)
				.name("ClusterLine frustum culling output")
			) }
		, m_frustumCullingOutputTerrainSRV{ device.createBufferSRV(m_frustumCullingOutputTerrainUAV) }
    {
        m_clusterFrustumCreateArguments.cs.clusterFrustumDispatchArgs = m_clusterCullDispatchArgsUAV;
    }

    void FrustumCuller::instanceCull(
        CommandList& cmd,
        Camera& camera,
        ModelResources& modelResources,
        Vector2<int> virtualResolution,
        DepthPyramid* depthPyramid,
        ClusterDataLine& appendOutput,
        ClusterDataLine& drawOutput)
    {
        auto instanceCount = modelResources.instanceCount();
        if (instanceCount == 0)
            return;

        CPU_MARKER(cmd.api(), "Frustum instance culling");
        GPU_MARKER(cmd, "Frustum instance culling");

        engine::vector<Vector4f> frustumPlanes = extractFrustumPlanes(camera.projectionMatrix() * camera.viewMatrix());

        drawOutput.clustersSRV = appendOutput.clustersSRV;
        drawOutput.clustersUAV = appendOutput.clustersUAV;
        cmd.copyBuffer(appendOutput.clusterCount.buffer, drawOutput.clusterIndexUAV.resource().buffer(), 1);

        cmd.clearBuffer(m_instanceCountUAV);
        cmd.clearBuffer(m_allocationUAV);
        cmd.clearBuffer(m_allocationSingleUAV);

		m_instanceFrustum.cs.subMeshBoundingBoxes = m_gpuBuffers.subMeshBoundingBox();
		m_instanceFrustum.cs.lodBinding = m_gpuBuffers.lod();
		m_instanceFrustum.cs.instanceLodBinding = m_gpuBuffers.instanceLodBinding();
		m_instanceFrustum.cs.subMeshData = m_gpuBuffers.subMeshData();
		m_instanceFrustum.cs.transformHistory = m_gpuBuffers.instanceTransform();
		m_instanceFrustum.cs.depthSampler = m_depthSampler;

		//LOG("Using subMeshBoundingBoxes: %llu, lodBinding: %llu, instanceLodBinding: %llu, subMeshData: %llu, transformHistory: %llu",
		//	m_gpuBuffers.subMeshBoundingBox().resourceId(),
		//	m_gpuBuffers.lod().resourceId(),
		//	m_gpuBuffers.instanceLodBinding().resourceId(),
		//	m_gpuBuffers.subMeshData().resourceId(),
		//	m_gpuBuffers.instanceTransform().resourceId());

        // constants
        m_instanceFrustum.cs.plane0 = frustumPlanes[0];
        m_instanceFrustum.cs.plane1 = frustumPlanes[1];
        m_instanceFrustum.cs.plane2 = frustumPlanes[2];
        m_instanceFrustum.cs.plane3 = frustumPlanes[3];
        m_instanceFrustum.cs.plane4 = frustumPlanes[4];
        m_instanceFrustum.cs.plane5 = frustumPlanes[5];
        m_instanceFrustum.cs.viewMatrix = fromMatrix(camera.viewMatrix());
        m_instanceFrustum.cs.projectionMatrix = fromMatrix(camera.projectionMatrix());
        m_instanceFrustum.cs.cameraPosition = camera.position();
        m_instanceFrustum.cs.instanceCount.x = static_cast<uint32_t>(modelResources.instanceCount());
        m_instanceFrustum.cs.size = Vector2f{ static_cast<float>(virtualResolution.x), static_cast<float>(virtualResolution.y) };
        m_instanceFrustum.cs.inverseSize = Vector2f{ 1.0f / static_cast<float>(virtualResolution.x), 1.0f / static_cast<float>(virtualResolution.y) };
        m_instanceFrustum.cs.pow2size = Vector2f{ static_cast<float>(roundUpToPow2(virtualResolution.x)), static_cast<float>(roundUpToPow2(virtualResolution.y)) };
        m_instanceFrustum.cs.farPlaneDistance = camera.farPlane();

        // output
        m_instanceFrustum.cs.cullingOutput = m_frustumCullingOutputUAV;
        m_instanceFrustum.cs.clusterCountBuffer = appendOutput.clusterCount.uav;
        m_instanceFrustum.cs.instanceCountBuffer = m_instanceCountUAV;
        m_instanceFrustum.cs.outputAllocationShared = m_allocationUAV;
        m_instanceFrustum.cs.outputAllocationSharedSingle = m_allocationSingleUAV;

        // depth for instance occlusion
        m_instanceFrustum.cs.depthPyramid = depthPyramid != nullptr ? depthPyramid->depth() : TextureSRV();

        cmd.bindPipe(m_instanceFrustum);
        cmd.dispatch(roundUpToMultiple(instanceCount, 64) / 64, 1, 1);

        m_bufferMath.perform(
            cmd, 
            appendOutput.clusterCount.srv, 
            drawOutput.clusterIndexSRV, 
            drawOutput.clusterCount.uav, 
            BufferMathOperation::Subtraction, 1, 0, 0, 0);
    }

    void FrustumCuller::instanceCullNoDepth(
        CommandList& cmd,
        const Camera& camera,
        ModelResources& modelResources,
        Vector2<int> virtualResolution,
        ClusterDataLine& appendOutput,
        ClusterDataLine& drawOutput)
    {
        CPU_MARKER(cmd.api(), "Frustum instance culling no depth");
        GPU_MARKER(cmd, "Frustum instance culling no depth");

        engine::vector<Vector4f> frustumPlanes = extractFrustumPlanes(camera.projectionMatrix() * camera.viewMatrix());

        drawOutput.clustersSRV = appendOutput.clustersSRV;
        drawOutput.clustersUAV = appendOutput.clustersUAV;
        cmd.copyBuffer(appendOutput.clusterCount.buffer, drawOutput.clusterIndexUAV.resource().buffer(), 1);

        cmd.clearBuffer(m_instanceCountUAV);
        cmd.clearBuffer(m_allocationUAV);
        cmd.clearBuffer(m_allocationSingleUAV);

        if (modelResources.instanceCount())
        {
            m_instanceFrustumNoDepth.cs.subMeshBoundingBoxes = m_gpuBuffers.subMeshBoundingBox();
            m_instanceFrustumNoDepth.cs.lodBinding = m_gpuBuffers.lod();
            m_instanceFrustumNoDepth.cs.instanceLodBinding = m_gpuBuffers.instanceLodBinding();
            m_instanceFrustumNoDepth.cs.subMeshData = m_gpuBuffers.subMeshData();
            m_instanceFrustumNoDepth.cs.transformHistory = m_gpuBuffers.instanceTransform();

            // constants
            m_instanceFrustumNoDepth.cs.plane0 = frustumPlanes[0];
            m_instanceFrustumNoDepth.cs.plane1 = frustumPlanes[1];
            m_instanceFrustumNoDepth.cs.plane2 = frustumPlanes[2];
            m_instanceFrustumNoDepth.cs.plane3 = frustumPlanes[3];
            m_instanceFrustumNoDepth.cs.plane4 = frustumPlanes[4];
            m_instanceFrustumNoDepth.cs.plane5 = frustumPlanes[5];
            m_instanceFrustumNoDepth.cs.viewMatrix = fromMatrix(camera.viewMatrix());
            m_instanceFrustumNoDepth.cs.projectionMatrix = fromMatrix(camera.projectionMatrix());
            m_instanceFrustumNoDepth.cs.cameraPosition = camera.position();
            m_instanceFrustumNoDepth.cs.instanceCount.x = static_cast<uint32_t>(modelResources.instanceCount());
            m_instanceFrustumNoDepth.cs.size = Vector2f{ static_cast<float>(virtualResolution.x), static_cast<float>(virtualResolution.y) };
            m_instanceFrustumNoDepth.cs.inverseSize = Vector2f{ 1.0f / static_cast<float>(virtualResolution.x), 1.0f / static_cast<float>(virtualResolution.y) };
            m_instanceFrustumNoDepth.cs.pow2size = Vector2f{ static_cast<float>(roundUpToPow2(virtualResolution.x)), static_cast<float>(roundUpToPow2(virtualResolution.y)) };
            m_instanceFrustumNoDepth.cs.farPlaneDistance = camera.farPlane();

            // output
            m_instanceFrustumNoDepth.cs.cullingOutput = m_frustumCullingOutputUAV;
            m_instanceFrustumNoDepth.cs.clusterCountBuffer = appendOutput.clusterCount.uav;
            m_instanceFrustumNoDepth.cs.instanceCountBuffer = m_instanceCountUAV;
            m_instanceFrustumNoDepth.cs.outputAllocationShared = m_allocationUAV;
            m_instanceFrustumNoDepth.cs.outputAllocationSharedSingle = m_allocationSingleUAV;

            cmd.bindPipe(m_instanceFrustumNoDepth);
            cmd.dispatch(roundUpToMultiple(modelResources.instanceCount(), 64) / 64, 1, 1);
        }

        m_bufferMath.perform(
            cmd,
            appendOutput.clusterCount.srv,
            drawOutput.clusterIndexSRV,
            drawOutput.clusterCount.uav,
            BufferMathOperation::Subtraction, 1, 0, 0, 0);
    }

    void FrustumCuller::instanceShadowCull(
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
        bool forceShadowPass)
    {
        if (modelResources.instanceCount() == 0)
            return;

        CPU_MARKER(cmd.api(), "Frustum shadow instance culling");
        GPU_MARKER(cmd, "Frustum shadow instance culling");

        engine::vector<Vector4f> frustumPlanes = extractFrustumPlanes(camera.projectionMatrix() * camera.viewMatrix());

        cmd.clearBuffer(m_instanceCountUAV);
        cmd.clearBuffer(m_allocationUAV);
        cmd.clearBuffer(m_allocationSingleUAV);

        cmd.clearBuffer(m_instanceCountAlphaClipUAV);
        cmd.clearBuffer(m_allocationAlphaClipUAV);
        cmd.clearBuffer(m_allocationAlphaClipSingleUAV);

		cmd.clearBuffer(m_instanceCountTerrainUAV);
		cmd.clearBuffer(m_allocationTerrainUAV);
        cmd.clearBuffer(m_allocationTerrainSingleUAV);

        cmd.clearBuffer(matchCount);

		m_instanceShadowFrustum.cs.subMeshBoundingBoxes = m_gpuBuffers.subMeshBoundingBox();
		m_instanceShadowFrustum.cs.lodBinding = m_gpuBuffers.lod();
		m_instanceShadowFrustum.cs.instanceLodBinding = m_gpuBuffers.instanceLodBinding();
		m_instanceShadowFrustum.cs.subMeshData = m_gpuBuffers.subMeshData();
		m_instanceShadowFrustum.cs.transformHistory = m_gpuBuffers.instanceTransform();
		m_instanceShadowFrustum.cs.instanceMaterials = m_gpuBuffers.instanceMaterial();

        // constants
        m_instanceShadowFrustum.cs.plane0 = frustumPlanes[0];
        m_instanceShadowFrustum.cs.plane1 = frustumPlanes[1];
        m_instanceShadowFrustum.cs.plane2 = frustumPlanes[2];
        m_instanceShadowFrustum.cs.plane3 = frustumPlanes[3];
        m_instanceShadowFrustum.cs.plane4 = frustumPlanes[4];
        m_instanceShadowFrustum.cs.plane5 = frustumPlanes[5];
        m_instanceShadowFrustum.cs.viewMatrix = fromMatrix(camera.viewMatrix());
        m_instanceShadowFrustum.cs.projectionMatrix = fromMatrix(camera.projectionMatrix());
        m_instanceShadowFrustum.cs.cameraPosition = camera.position();
        m_instanceShadowFrustum.cs.instanceCount.x = static_cast<uint32_t>(modelResources.instanceCount());
        m_instanceShadowFrustum.cs.size = Vector2f{ static_cast<float>(virtualResolution.x), static_cast<float>(virtualResolution.y) };
        m_instanceShadowFrustum.cs.inverseSize = Vector2f{ 1.0f / static_cast<float>(virtualResolution.x), 1.0f / static_cast<float>(virtualResolution.y) };
        m_instanceShadowFrustum.cs.pow2size = Vector2f{ static_cast<float>(roundUpToPow2(virtualResolution.x)), static_cast<float>(roundUpToPow2(virtualResolution.y)) };
        m_instanceShadowFrustum.cs.farPlaneDistance = camera.farPlane();

        // output
        m_instanceShadowFrustum.cs.cullingOutput = m_frustumCullingOutputUAV;
        m_instanceShadowFrustum.cs.clusterCountBuffer = output.clusterCount.uav;
        m_instanceShadowFrustum.cs.instanceCountBuffer = m_instanceCountUAV;
        m_instanceShadowFrustum.cs.outputAllocationShared = m_allocationUAV;
        m_instanceShadowFrustum.cs.outputAllocationSharedSingle = m_allocationSingleUAV;

        m_instanceShadowFrustum.cs.cullingOutputAlphaClip = m_frustumCullingOutputAlphaClipUAV;
        m_instanceShadowFrustum.cs.clusterCountBufferAlphaClip = outputAlphaClipped.clusterCount.uav;
        m_instanceShadowFrustum.cs.instanceCountBufferAlphaClip = m_instanceCountAlphaClipUAV;
        m_instanceShadowFrustum.cs.outputAllocationSharedAlphaClip = m_allocationAlphaClipUAV;
        m_instanceShadowFrustum.cs.outputAllocationSharedAlphaClipSingle = m_allocationAlphaClipSingleUAV;

		m_instanceShadowFrustum.cs.cullingOutputTerrain = m_frustumCullingOutputTerrainUAV;
		m_instanceShadowFrustum.cs.clusterCountBufferTerrain = outputTerrain.clusterCount.uav;
		m_instanceShadowFrustum.cs.instanceCountBufferTerrain = m_instanceCountTerrainUAV;
		m_instanceShadowFrustum.cs.outputAllocationSharedTerrain = m_allocationTerrainUAV;
        m_instanceShadowFrustum.cs.outputAllocationSharedTerrainSingle = m_allocationTerrainSingleUAV;

        m_instanceShadowFrustum.cs.sceneChange = sceneChanges;
        m_instanceShadowFrustum.cs.changedCounter = changeCount;
        m_instanceShadowFrustum.cs.matches = matchCount;
        m_instanceShadowFrustum.cs.force.x = forceShadowPass ? 1u : 0u;

        cmd.bindPipe(m_instanceShadowFrustum);
        cmd.dispatch(roundUpToMultiple(modelResources.instanceCount(), 64) / 64, 1, 1);
    }

    void FrustumCuller::expandClusters(
        CommandList& cmd, 
        ClusterDataLine& output)
    {
        m_clusterExpansion->expandClusters(
            cmd,
            m_frustumCullingOutputSRV,
            m_instanceCountSRV,
            output);
    }

    void FrustumCuller::expandClustersShadowAlphaClip(
        CommandList& cmd,
        ClusterDataLine& output)
    {
        m_clusterExpansion->expandClusters(
            cmd,
            m_frustumCullingOutputAlphaClipSRV,
            m_instanceCountAlphaClipSRV,
            output);
    }

	void FrustumCuller::expandClustersShadowTerrain(
		CommandList& cmd,
		ClusterDataLine& output)
	{
		m_clusterExpansion->expandClusters(
			cmd,
			m_frustumCullingOutputTerrainSRV,
			m_instanceCountTerrainSRV,
			output);
	}

    void FrustumCuller::clearInstanceCount(CommandList& cmd)
    {
        cmd.clearBuffer(m_instanceCountUAV);
    }

    void FrustumCuller::clusterCull(
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
        BufferUAV clusterTrackingClusters)
    {
        CPU_MARKER(cmd.api(), "1. Cluster frustum cull");
        GPU_MARKER(cmd, "1. Cluster frustum cull");
        {
            CPU_MARKER(cmd.api(), "Create cluster frustum cull arguments");
            GPU_MARKER(cmd, "Create cluster frustum cull arguments");

            m_clusterFrustumCreateArguments.cs.clusterCountBuffer = input.clusterCount.srv;
            cmd.bindPipe(m_clusterFrustumCreateArguments);
            cmd.dispatch(1, 1, 1);
        }
        {
            CPU_MARKER(cmd.api(), "Cluster frustum cull");
            GPU_MARKER(cmd, "Cluster frustum cull");

            drawOutput.clustersSRV = appendOutput.clustersSRV;
            drawOutput.clustersUAV = appendOutput.clustersUAV;
            cmd.copyBuffer(appendOutput.clusterCount.buffer, drawOutput.clusterIndexUAV.resource().buffer(), 1);

            drawAlphaclippedOutput.clustersSRV = appendAlphaclippedOutput.clustersSRV;
            drawAlphaclippedOutput.clustersUAV = appendAlphaclippedOutput.clustersUAV;
            cmd.copyBuffer(appendAlphaclippedOutput.clusterCount.buffer, drawAlphaclippedOutput.clusterIndexUAV.resource().buffer(), 1);

            drawTransparentOutput.clustersSRV = appendTransparentOutput.clustersSRV;
            drawTransparentOutput.clustersUAV = appendTransparentOutput.clustersUAV;
            cmd.copyBuffer(appendTransparentOutput.clusterCount.buffer, drawTransparentOutput.clusterIndexUAV.resource().buffer(), 1);

			drawTerrainOutput.clustersSRV = appendTerrainOutput.clustersSRV;
			drawTerrainOutput.clustersUAV = appendTerrainOutput.clustersUAV;
			cmd.copyBuffer(appendTerrainOutput.clusterCount.buffer, drawTerrainOutput.clusterIndexUAV.resource().buffer(), 1);

            engine::vector<Vector4f> frustumPlanes = extractFrustumPlanes(camera.projectionMatrix() * camera.viewMatrix());

			m_clusterFrustum.cs.subMeshBoundingBoxes = m_gpuBuffers.subMeshBoundingBox();
			m_clusterFrustum.cs.lodBinding = m_gpuBuffers.lod();
			m_clusterFrustum.cs.instanceLodBinding = m_gpuBuffers.instanceLodBinding();
			m_clusterFrustum.cs.transformHistory = m_gpuBuffers.instanceTransform();
			m_clusterFrustum.cs.instanceMaterials = m_gpuBuffers.instanceMaterial();

            // constants
            m_clusterFrustum.cs.plane0 = frustumPlanes[0];
            m_clusterFrustum.cs.plane1 = frustumPlanes[1];
            m_clusterFrustum.cs.plane2 = frustumPlanes[2];
            m_clusterFrustum.cs.plane3 = frustumPlanes[3];
            m_clusterFrustum.cs.plane4 = frustumPlanes[4];
            m_clusterFrustum.cs.plane5 = frustumPlanes[5];
            m_clusterFrustum.cs.viewMatrix = fromMatrix(camera.viewMatrix());
            m_clusterFrustum.cs.projectionMatrix = fromMatrix(camera.projectionMatrix());
            m_clusterFrustum.cs.cameraPosition = camera.position();
            m_clusterFrustum.cs.instanceCount.x = static_cast<uint32_t>(modelResources.instanceCount());
            m_clusterFrustum.cs.size = Vector2f{ static_cast<float>(virtualResolution.x), static_cast<float>(virtualResolution.y) };
            m_clusterFrustum.cs.inverseSize = Vector2f{ 1.0f / static_cast<float>(virtualResolution.x), 1.0f / static_cast<float>(virtualResolution.y) };
            m_clusterFrustum.cs.fnvPrime.x = 16777619u;
            m_clusterFrustum.cs.fnvOffsetBasis.x = 2166136261u;
            m_clusterFrustum.cs.farPlaneDistance = camera.farPlane();

            // inputs
            m_clusterFrustum.cs.inputClusters = input.clustersSRV;
            m_clusterFrustum.cs.inputClusterCount = input.clusterCount.srv;
            m_clusterFrustum.cs.inputClusterIndex = input.clusterIndexSRV;

            // outputs
            m_clusterFrustum.cs.outputClusters = appendOutput.clustersUAV;
            m_clusterFrustum.cs.outputClusterDistributor = appendOutput.clusterCount.uav;
            m_clusterFrustum.cs.outputClusterIndex = appendOutput.clusterIndexSRV;

            // alphaclipped output
            m_clusterFrustum.cs.alphaClippedOutputClusters = appendAlphaclippedOutput.clustersUAV;
            m_clusterFrustum.cs.alphaClippedOutputClusterDistributor = appendAlphaclippedOutput.clusterCount.uav;
            m_clusterFrustum.cs.alphaClippedOutputClusterIndex = appendAlphaclippedOutput.clusterIndexSRV;

            // transparent output
            m_clusterFrustum.cs.transparentOutputClusters = appendTransparentOutput.clustersUAV;
            m_clusterFrustum.cs.transparentOutputClusterDistributor = appendTransparentOutput.clusterCount.uav;
            m_clusterFrustum.cs.transparentOutputClusterIndex = appendTransparentOutput.clusterIndexSRV;

			// terrain output
			m_clusterFrustum.cs.terrainOutputClusters = appendTerrainOutput.clustersUAV;
			m_clusterFrustum.cs.terrainOutputClusterDistributor = appendTerrainOutput.clusterCount.uav;
			m_clusterFrustum.cs.terrainOutputClusterIndex = appendTerrainOutput.clusterIndexSRV;

            // cluster tracking
            m_clusterFrustum.cs.clusterAccurateTrackingInstancePtr = clusterTrackingInstanceToCluster;
            m_clusterFrustum.cs.clusterAccurateTracking = clusterTrackingClusters;
            m_clusterFrustum.cs.subMeshData = m_device.modelResources().gpuBuffers().subMeshData();

            cmd.bindPipe(m_clusterFrustum);
            cmd.dispatchIndirect(m_clusterCullDispatchArgs, 0);

            m_bufferMathSecond.perform(
                cmd,
                appendOutput.clusterCount.srv,
                drawOutput.clusterIndexSRV,
                drawOutput.clusterCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

            m_bufferMathSecond.perform(
                cmd,
                appendAlphaclippedOutput.clusterCount.srv,
                drawAlphaclippedOutput.clusterIndexSRV,
                drawAlphaclippedOutput.clusterCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

            m_bufferMathSecond.perform(
                cmd,
                appendTransparentOutput.clusterCount.srv,
                drawTransparentOutput.clusterIndexSRV,
                drawTransparentOutput.clusterCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

			m_bufferMathSecond.perform(
				cmd,
				appendTerrainOutput.clusterCount.srv,
				drawTerrainOutput.clusterIndexSRV,
				drawTerrainOutput.clusterCount.uav,
				BufferMathOperation::Subtraction, 1, 0, 0, 0);
        }
    }
}
