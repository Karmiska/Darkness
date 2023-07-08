#include "engine/rendering/renderers/RenderGbuffer.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/rendering/GBuffer.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/SamplerDescription.h"
#include "components/Camera.h"
#include "shaders/core/shared_types/DebugModes.hlsli"

namespace engine
{
	template<typename T>
	void setupPipeline(
		Device& device,
		T& pipeline,
		DepthTestOption depthTest,
		CullMode cullMode,
		BiasOption bias,
        DebugOption debug)
	{
		pipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		pipeline.setRasterizerState(RasterizerDescription().cullMode(cullMode));
        pipeline.vs.debug = debug == DebugOption::Enabled;
		pipeline.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(depthTest == DepthTestOption::GreaterEqual ? ComparisonFunction::GreaterEqual : ComparisonFunction::Equal));

		if (bias == BiasOption::Enabled)
		{
			pipeline.setRasterizerState(RasterizerDescription()
				.depthBias(-16384)
				.depthBiasClamp(1000000.0f)
				.slopeScaledDepthBias(-1.75f)
				.cullMode(CullMode::None));
		}

		// TODO: these aren't necessary for opaque stuff
		pipeline.vs.uv = device.modelResources().gpuBuffers().uv();
		pipeline.ps.instanceMaterials = device.modelResources().gpuBuffers().instanceMaterial();
		pipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear));
        pipeline.ps.debug = debug == DebugOption::Enabled;
	}

	RenderGbuffer::RenderGbuffer(Device& device)
		: m_device{ device }
		, m_default{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_alphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
		, m_jitter{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_jitterAlphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
		, m_defaultBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_alphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
		, m_jitterBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_jitterAlphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
		, m_equaldefault{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_equalalphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
		, m_equaljitter{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_equaljitterAlphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
		, m_equaldefaultBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_equalalphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
		, m_equaljitterBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
		, m_equaljitterAlphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugDefault{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugAlphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugJitter{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugJitterAlphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugDefaultBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugAlphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugJitterBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugJitterAlphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugEqualdefault{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugEqualalphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugEqualjitter{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugEqualjitterAlphaClipped{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugEqualdefaultBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugEqualalphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
        , m_debugEqualjitterBias{ device.createPipeline<shaders::RenderClustersGbuffer>() }
        , m_debugEqualjitterAlphaClippedBias{ device.createPipeline<shaders::RenderClustersGbufferAlphaClipped>() }
	{
		setupPipeline(device, m_default,						DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_alphaClipped,					DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_jitter,							DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_jitterAlphaClipped,				DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_defaultBias,					DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Enabled, DebugOption::Disabled);
		setupPipeline(device, m_alphaClippedBias,				DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Enabled, DebugOption::Disabled);
		setupPipeline(device, m_jitterBias,						DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Enabled, DebugOption::Disabled);
		setupPipeline(device, m_jitterAlphaClippedBias,			DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Enabled, DebugOption::Disabled);

		setupPipeline(device, m_equaldefault,					DepthTestOption::Equal, CullMode::Back, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_equalalphaClipped,				DepthTestOption::Equal, CullMode::None, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_equaljitter,					DepthTestOption::Equal, CullMode::Back, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_equaljitterAlphaClipped,		DepthTestOption::Equal, CullMode::None, BiasOption::Disabled, DebugOption::Disabled);
		setupPipeline(device, m_equaldefaultBias,				DepthTestOption::Equal, CullMode::Back, BiasOption::Enabled, DebugOption::Disabled);
		setupPipeline(device, m_equalalphaClippedBias,			DepthTestOption::Equal, CullMode::None, BiasOption::Enabled, DebugOption::Disabled);
		setupPipeline(device, m_equaljitterBias,				DepthTestOption::Equal, CullMode::Back, BiasOption::Enabled, DebugOption::Disabled);
		setupPipeline(device, m_equaljitterAlphaClippedBias,	DepthTestOption::Equal, CullMode::None, BiasOption::Enabled, DebugOption::Disabled);

        setupPipeline(device, m_debugDefault,						DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugAlphaClipped,					DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugJitter,							DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugJitterAlphaClipped,				DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugDefaultBias,					DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Enabled, DebugOption::Enabled);
		setupPipeline(device, m_debugAlphaClippedBias,				DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Enabled, DebugOption::Enabled);
		setupPipeline(device, m_debugJitterBias,						DepthTestOption::GreaterEqual, CullMode::Back, BiasOption::Enabled, DebugOption::Enabled);
		setupPipeline(device, m_debugJitterAlphaClippedBias,			DepthTestOption::GreaterEqual, CullMode::None, BiasOption::Enabled, DebugOption::Enabled);

		setupPipeline(device, m_debugEqualdefault,					DepthTestOption::Equal, CullMode::Back, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugEqualalphaClipped,				DepthTestOption::Equal, CullMode::None, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugEqualjitter,					DepthTestOption::Equal, CullMode::Back, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugEqualjitterAlphaClipped,		DepthTestOption::Equal, CullMode::None, BiasOption::Disabled, DebugOption::Enabled);
		setupPipeline(device, m_debugEqualdefaultBias,				DepthTestOption::Equal, CullMode::Back, BiasOption::Enabled, DebugOption::Enabled);
		setupPipeline(device, m_debugEqualalphaClippedBias,			DepthTestOption::Equal, CullMode::None, BiasOption::Enabled, DebugOption::Enabled);
		setupPipeline(device, m_debugEqualjitterBias,				DepthTestOption::Equal, CullMode::Back, BiasOption::Enabled, DebugOption::Enabled);
		setupPipeline(device, m_debugEqualjitterAlphaClippedBias,	DepthTestOption::Equal, CullMode::None, BiasOption::Enabled, DebugOption::Enabled);
	}

	template<typename T>
	void setupRender(
		Device& device,
		T& pipeline, 
		BufferSRV clusters, 
		Camera& camera,
		const Vector2<int>& virtualResolution,
		const Matrix4f& previousCameraViewMatrix,
		const Matrix4f& previousCameraProjectionMatrix,
        int debug)
	{
		pipeline.vs.vertices = device.modelResources().gpuBuffers().vertex();
		pipeline.vs.normals = device.modelResources().gpuBuffers().normal();
		pipeline.vs.tangents = device.modelResources().gpuBuffers().tangent();
		pipeline.vs.uv = device.modelResources().gpuBuffers().uv();
		pipeline.vs.scales = device.modelResources().gpuBuffers().instanceScale();
		pipeline.vs.clusterData = device.modelResources().gpuBuffers().clusterBinding();
		pipeline.vs.transformHistory = device.modelResources().gpuBuffers().instanceTransform();

		pipeline.vs.clusters = clusters;
		pipeline.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
		pipeline.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
		pipeline.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
		pipeline.ps.materialTextures = device.modelResources().textures();

        if ((debug == DEBUG_MODE_CLUSTERS) ||
            (debug == DEBUG_MODE_TRIANGLES))
        {
            pipeline.vs.debug = true;
            pipeline.ps.debug = true;
        }
        else
        {
            pipeline.vs.debug = false;
            pipeline.ps.debug = false;
        }
        pipeline.vs.debugMode.x = debug;
    }

	void RenderGbuffer::render(
		CommandList& cmd,
		GBuffer& gBuffer,
		ClusterDataLine& clusters,
		IndexDataLine& indexes,
		DrawDataLine& draws,
		TextureDSV depth,
		Camera& camera,
		JitterOption jitter,
		AlphaClipOption alphaClipped,
        int debugMode,
		BiasOption bias,
		DepthTestOption depthTest,
		const Vector2<int>& virtualResolution,
		const Matrix4f& previousCameraViewMatrix,
		const Matrix4f& previousCameraProjectionMatrix)
	{
		cmd.setRenderTargets({
			gBuffer.rtv(GBufferType::Normal),
			gBuffer.rtv(GBufferType::Uv),
			gBuffer.rtv(GBufferType::Motion),
			gBuffer.rtv(GBufferType::InstanceId) }, depth);

        if ((debugMode != DEBUG_MODE_CLUSTERS) &&
            (debugMode != DEBUG_MODE_TRIANGLES))
        {
            if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_default, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_default);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_alphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_alphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_jitter, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_jitter);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_jitterAlphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_jitterAlphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_defaultBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_defaultBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_alphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_alphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_jitterBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_jitterBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_jitterAlphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_jitterAlphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_equaldefault, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equaldefault);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_equalalphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equalalphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_equaljitter, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equaljitter);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_equaljitterAlphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equaljitterAlphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_equaldefaultBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equaldefaultBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_equalalphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equalalphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_equaljitterBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equaljitterBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_equaljitterAlphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_equaljitterAlphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
        }
        else
        {
            if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugDefault, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugDefault);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugAlphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugAlphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugJitter, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugJitter);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugJitterAlphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugJitterAlphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugDefaultBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugDefaultBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugAlphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugAlphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugJitterBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugJitterBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::GreaterEqual)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = greaterEqual)");

                setupRender(m_device, m_debugJitterAlphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugJitterAlphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualdefault, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualdefault);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualalphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualalphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualjitter, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualjitter);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Disabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualjitterAlphaClipped, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualjitterAlphaClipped);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualdefaultBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualdefaultBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Disabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualalphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualalphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Disabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualjitterBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualjitterBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
            else if (jitter == JitterOption::Enabled &&
                alphaClipped == AlphaClipOption::Enabled &&
                bias == BiasOption::Enabled &&
                depthTest == DepthTestOption::Equal)
            {
                CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");
                GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF, depth = equal)");

                setupRender(m_device, m_debugEqualjitterAlphaClippedBias, clusters.clustersSRV, camera, virtualResolution, previousCameraViewMatrix, previousCameraProjectionMatrix, debugMode);
                cmd.bindPipe(m_debugEqualjitterAlphaClippedBias);
                cmd.executeIndexedIndirect(
                    indexes.indexBuffer,
                    draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
                    draws.clusterRendererExecuteIndirectCount.buffer, 0);
            }
        }
	}

}
