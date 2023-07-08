#include "engine/rendering/renderers/RenderForward.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/rendering/GBuffer.h"
#include "engine/rendering/LightData.h"
#include "engine/rendering/ShapeRenderer.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/SamplerDescription.h"
#include "components/Camera.h"

namespace engine
{
	enum class PipelineMode
	{
		Default,
		Debug
	};

	template<typename T>
	void setupPipeline(
		Device& device,
		T& pipeline,
		DepthTestOption depthTest,
		DepthStencilOpDescription front,
		DepthStencilOpDescription back,
		bool cullBack,
		PipelineMode mode = PipelineMode::Default)
	{
		pipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		pipeline.setRasterizerState(RasterizerDescription().cullMode(cullBack ? CullMode::Back : CullMode::None));
		pipeline.ps.debug = mode == PipelineMode::Debug;
		pipeline.vs.debug = mode == PipelineMode::Debug;
		pipeline.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true).depthWriteMask(depthTest == DepthTestOption::GreaterEqual ? DepthWriteMask::All : DepthWriteMask::Zero)
			.depthFunc(depthTest == DepthTestOption::GreaterEqual ? ComparisonFunction::GreaterEqual : ComparisonFunction::Equal)
			.frontFace(front).backFace(back));
		pipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
		pipeline.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
		pipeline.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
		pipeline.ps.shadow_sampler = device.createSampler(SamplerDescription()
			.addressU(TextureAddressMode::Mirror)
			.addressV(TextureAddressMode::Mirror)
			.filter(Filter::Comparison));
	};

	RenderForward::RenderForward(Device& device)
		: m_device{ device }

		, m_defaultGreaterEqual{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_defaultGreaterEqualDebug{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_jitterGreaterEqual{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_jitterGreaterEqualDebug{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_alphaClippedGreaterEqual{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_alphaClippedGreaterEqualDebug{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_jitterAlphaClippedGreaterEqual{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_jitterAlphaClippedGreaterEqualDebug{ device.createPipeline<shaders::RenderClustersForward>() }

		, m_defaultEqual{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_defaultEqualDebug{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_jitterEqual{ device.createPipeline<shaders::RenderClustersForwardJitterEqual>() }
		, m_jitterEqualDebug{ device.createPipeline<shaders::RenderClustersForwardJitterEqual>() }
		, m_alphaClippedEqual{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_alphaClippedEqualDebug{ device.createPipeline<shaders::RenderClustersForward>() }
		, m_jitterAlphaClippedEqual{ device.createPipeline<shaders::RenderClustersForwardJitterAlphaClippedEqual>() }
		, m_jitterAlphaClippedEqualDebug{ device.createPipeline<shaders::RenderClustersForwardJitterAlphaClippedEqual>() }
	{
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

		setupPipeline(m_device, m_defaultGreaterEqual, DepthTestOption::GreaterEqual, front, back, true);
		setupPipeline(m_device, m_defaultGreaterEqualDebug, DepthTestOption::GreaterEqual, front, back, true, PipelineMode::Debug);
		setupPipeline(m_device, m_jitterGreaterEqual, DepthTestOption::GreaterEqual, front, back, true);
		setupPipeline(m_device, m_jitterGreaterEqualDebug, DepthTestOption::GreaterEqual, front, back, true, PipelineMode::Debug);
		setupPipeline(m_device, m_alphaClippedGreaterEqual, DepthTestOption::GreaterEqual, front, back, false);
		setupPipeline(m_device, m_alphaClippedGreaterEqualDebug, DepthTestOption::GreaterEqual, front, back, false, PipelineMode::Debug);
		setupPipeline(m_device, m_jitterAlphaClippedGreaterEqual, DepthTestOption::GreaterEqual, front, back, false);
		setupPipeline(m_device, m_jitterAlphaClippedGreaterEqualDebug, DepthTestOption::GreaterEqual, front, back, false, PipelineMode::Debug);

		setupPipeline(m_device, m_defaultEqual, DepthTestOption::Equal, front, back, true);
		setupPipeline(m_device, m_defaultEqualDebug, DepthTestOption::Equal, front, back, true, PipelineMode::Debug);
		setupPipeline(m_device, m_jitterEqual, DepthTestOption::Equal, front, back, true);
		setupPipeline(m_device, m_jitterEqualDebug, DepthTestOption::Equal, front, back, true, PipelineMode::Debug);
		setupPipeline(m_device, m_alphaClippedEqual, DepthTestOption::Equal, front, back, false);
		setupPipeline(m_device, m_alphaClippedEqualDebug, DepthTestOption::Equal, front, back, false, PipelineMode::Debug);
		setupPipeline(m_device, m_jitterAlphaClippedEqual, DepthTestOption::Equal, front, back, false);
		setupPipeline(m_device, m_jitterAlphaClippedEqualDebug, DepthTestOption::Equal, front, back, false, PipelineMode::Debug);
	}

	template<typename T>
	void setupPipeline(
		Device& device,
		T& pipeline,
		Camera& camera,
		ClusterDataLine& clusters,
		TextureSRV shadowMap,
		BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
		TextureSRV ssao,
		int width, int height,
		Vector3f probePosition,
		float probeRange,
		LightData& lights,
		BufferSRV lightBins)
	{
		pipeline.vs.vertices = device.modelResources().gpuBuffers().vertex();
		pipeline.vs.normals = device.modelResources().gpuBuffers().normal();
		pipeline.vs.tangents = device.modelResources().gpuBuffers().tangent();
		pipeline.vs.uv = device.modelResources().gpuBuffers().uv();
		pipeline.vs.scales = device.modelResources().gpuBuffers().instanceScale();
		// pipeline.vs.clusters -> filled in later
		pipeline.vs.clusterData = device.modelResources().gpuBuffers().clusterBinding();
		pipeline.vs.transformHistory = device.modelResources().gpuBuffers().instanceTransform();

		pipeline.ps.instanceMaterials = device.modelResources().gpuBuffers().instanceMaterial();

		pipeline.vs.clusters = clusters.clustersSRV;

		pipeline.ps.cameraWorldSpacePosition = camera.position();
		
		pipeline.ps.environmentStrength = camera.environmentMapStrength();
		if (camera.environmentIrradiance().valid() && camera.environmentIrradiance().texture().arraySlices() == 1)
		{
			pipeline.ps.environmentIrradiance = camera.environmentIrradiance();
			pipeline.ps.environmentIrradianceCubemap = TextureSRV();
			pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(false);
			pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(true);
		}
		else
		{
			pipeline.ps.environmentIrradiance = TextureSRV();
			pipeline.ps.environmentIrradianceCubemap = camera.environmentIrradiance();
			pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(true);
			pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(false);
		}
		pipeline.ps.environmentSpecular = camera.environmentSpecular();
		pipeline.ps.environmentBrdfLut = camera.environmentBrdfLUT();
		pipeline.ps.hasEnvironmentSpecular.x = camera.environmentSpecular().valid();
		pipeline.ps.shadowSize = Float2{ 1.0f / static_cast<float>(ShadowMapWidth), 1.0f / static_cast<float>(ShadowMapHeight) };
		pipeline.ps.shadowMap = shadowMap;
		pipeline.ps.shadowVP = shadowVP;

		pipeline.ps.cameraInverseProjectionMatrix = fromMatrix(camera.projectionMatrix().inverse());
		pipeline.ps.cameraInverseViewMatrix = fromMatrix(camera.viewMatrix().inverse());

		pipeline.ps.ssao = ssao;
		pipeline.ps.resolution = Float3{ static_cast<float>(camera.width()), static_cast<float>(camera.height()), 1.0f };

		pipeline.ps.frameSize.x = width;
		pipeline.ps.frameSize.y = height;

		pipeline.ps.instanceMaterials = device.modelResources().gpuBuffers().instanceMaterial();
		pipeline.ps.materialTextures = device.modelResources().textures();

		pipeline.ps.probePositionRange = Float4{ probePosition.x, probePosition.y, probePosition.z, probeRange };
		pipeline.ps.probeBBmin = Float4{ probePosition.x - probeRange, probePosition.y - probeRange, probePosition.z - probeRange, 0.0f };
		pipeline.ps.probeBBmax = Float4{ probePosition.x + probeRange, probePosition.y + probeRange, probePosition.z + probeRange, 0.0f };

		pipeline.ps.usingProbe.x = 0;
		pipeline.ps.binSize.x = binSize(camera.width());
		pipeline.ps.binSize.y = binSize(camera.height());

		if (lights.count() > 0)
		{
			pipeline.ps.lightWorldPosition = lights.worldPositions();
			pipeline.ps.lightDirection = lights.directions();
			pipeline.ps.lightColor = lights.colors();
			pipeline.ps.lightIntensity = lights.intensities();
			pipeline.ps.lightRange = lights.ranges();
			pipeline.ps.lightType = lights.types();
			pipeline.ps.lightParameters = lights.parameters();
			pipeline.ps.lightBins = lightBins;
			pipeline.ps.lightIndexToShadowIndex = lightIndexToShadowIndex;
		}
		pipeline.ps.exposure = camera.exposure();
	};

	void RenderForward::render(
		CommandList& cmd,
		ClusterDataLine& clusters,
		IndexDataLine& indexes,
		DrawDataLine& draws,
		TextureDSV depth,
		TextureRTV rtv,
		TextureRTV motion,
		TextureSRV shadowMap,
		BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
		TextureSRV ssao,
		Camera& camera,
		Vector3f probePosition,
		float probeRange,
		JitterOption jitter,
		AlphaClipOption alphaClipped,
		DepthTestOption depthTest,
		uint32_t debugMode,
		const Vector2<int>& virtualResolution,
		LightData& lights,
		BufferSRV lightBins,
		const Matrix4f& previousCameraViewMatrix,
		const Matrix4f& previousCameraProjectionMatrix)
	{
		cmd.setRenderTargets({ rtv, motion }, depth);

		// TODO: Check these pipeline!
		//		 It looks like all of them setup jittering ???

		if (jitter == JitterOption::Disabled &&
			alphaClipped == AlphaClipOption::Disabled &&
			depthTest == DepthTestOption::GreaterEqual)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = OFF, alphaclip = OFF, depthTest = GreaterEqual)");
			GPU_MARKER(cmd, "Render forward (jitter = OFF, alphaclip = OFF, depthTest = GreaterEqual)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_defaultGreaterEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_defaultGreaterEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_defaultGreaterEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_defaultGreaterEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_defaultGreaterEqual.vs.debugMode.x = debugMode;
				m_defaultGreaterEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_defaultGreaterEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_defaultGreaterEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_defaultGreaterEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_defaultGreaterEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_defaultGreaterEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_defaultGreaterEqualDebug.vs.debugMode.x = debugMode;
				m_defaultGreaterEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_defaultGreaterEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}
		else if (jitter == JitterOption::Enabled &&
				 alphaClipped == AlphaClipOption::Disabled &&
				 depthTest == DepthTestOption::GreaterEqual)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = ON, alphaclip = OFF, depthTest = GreaterEqual)");
			GPU_MARKER(cmd, "Render forward (jitter = ON, alphaclip = OFF, depthTest = GreaterEqual)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_jitterGreaterEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterGreaterEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterGreaterEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterGreaterEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterGreaterEqual.vs.debugMode.x = debugMode;
				m_jitterGreaterEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterGreaterEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_jitterGreaterEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterGreaterEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterGreaterEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterGreaterEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterGreaterEqualDebug.vs.debugMode.x = debugMode;
				m_jitterGreaterEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterGreaterEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}
		else if (jitter == JitterOption::Disabled &&
			alphaClipped == AlphaClipOption::Enabled &&
			depthTest == DepthTestOption::GreaterEqual)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = OFF, alphaclip = ON, depthTest = GreaterEqual)");
			GPU_MARKER(cmd, "Render forward (jitter = OFF, alphaclip = ON, depthTest = GreaterEqual)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_alphaClippedGreaterEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_alphaClippedGreaterEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedGreaterEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_alphaClippedGreaterEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedGreaterEqual.vs.debugMode.x = debugMode;
				m_alphaClippedGreaterEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_alphaClippedGreaterEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_alphaClippedGreaterEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_alphaClippedGreaterEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedGreaterEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_alphaClippedGreaterEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedGreaterEqualDebug.vs.debugMode.x = debugMode;
				m_alphaClippedGreaterEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_alphaClippedGreaterEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}
		else if (jitter == JitterOption::Enabled &&
			alphaClipped == AlphaClipOption::Enabled &&
			depthTest == DepthTestOption::GreaterEqual)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = ON, alphaclip = ON, depthTest = GreaterEqual)");
			GPU_MARKER(cmd, "Render forward (jitter = ON, alphaclip = ON, depthTest = GreaterEqual)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_jitterAlphaClippedGreaterEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterAlphaClippedGreaterEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedGreaterEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterAlphaClippedGreaterEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedGreaterEqual.vs.debugMode.x = debugMode;
				m_jitterAlphaClippedGreaterEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterAlphaClippedGreaterEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_jitterAlphaClippedGreaterEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterAlphaClippedGreaterEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedGreaterEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterAlphaClippedGreaterEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedGreaterEqualDebug.vs.debugMode.x = debugMode;
				m_jitterAlphaClippedGreaterEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterAlphaClippedGreaterEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}
		else if (jitter == JitterOption::Disabled &&
				alphaClipped == AlphaClipOption::Disabled &&
				depthTest == DepthTestOption::Equal)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = OFF, alphaclip = OFF, depthTest = Equal)");
			GPU_MARKER(cmd, "Render forward (jitter = OFF, alphaclip = OFF, depthTest = Equal)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_defaultEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_defaultEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_defaultEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_defaultEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_defaultEqual.vs.debugMode.x = debugMode;
				m_defaultEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_defaultEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_defaultEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_defaultEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_defaultEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_defaultEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_defaultEqualDebug.vs.debugMode.x = debugMode;
				m_defaultEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_defaultEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}

		else if (jitter == JitterOption::Enabled &&
				 alphaClipped == AlphaClipOption::Disabled &&
				 depthTest == DepthTestOption::Equal)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = ON, alphaclip = OFF, depthTest = Equal)");
			GPU_MARKER(cmd, "Render forward (jitter = ON, alphaclip = OFF, depthTest = Equal)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_jitterEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterEqual.vs.debugMode.x = debugMode;
				m_jitterEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_jitterEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterEqualDebug.vs.debugMode.x = debugMode;
				m_jitterEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}
		else if (jitter == JitterOption::Disabled &&
			alphaClipped == AlphaClipOption::Enabled &&
			depthTest == DepthTestOption::Equal)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = OFF, alphaclip = ON, depthTest = Equal)");
			GPU_MARKER(cmd, "Render forward (jitter = OFF, alphaclip = ON, depthTest = Equal)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_alphaClippedEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_alphaClippedEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_alphaClippedEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedEqual.vs.debugMode.x = debugMode;
				m_alphaClippedEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_alphaClippedEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_alphaClippedEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_alphaClippedEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_alphaClippedEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_alphaClippedEqualDebug.vs.debugMode.x = debugMode;
				m_alphaClippedEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_alphaClippedEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}
		else if (jitter == JitterOption::Enabled &&
			alphaClipped == AlphaClipOption::Enabled &&
			depthTest == DepthTestOption::Equal)
		{
			CPU_MARKER(cmd.api(), "Render forward (jitter = ON, alphaclip = ON, depthTest = Equal)");
			GPU_MARKER(cmd, "Render forward (jitter = ON, alphaclip = ON, depthTest = Equal)");

			if (debugMode == 0)
			{
				setupPipeline(m_device, m_jitterAlphaClippedEqual, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterAlphaClippedEqual.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedEqual.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterAlphaClippedEqual.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedEqual.vs.debugMode.x = debugMode;
				m_jitterAlphaClippedEqual.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterAlphaClippedEqual);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
			else
			{
				setupPipeline(m_device, m_jitterAlphaClippedEqualDebug, camera, clusters, shadowMap, shadowVP, lightIndexToShadowIndex, ssao, virtualResolution.x, virtualResolution.y, probePosition, probeRange, lights, lightBins);
				m_jitterAlphaClippedEqualDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedEqualDebug.vs.previousViewProjectionMatrix = fromMatrix(previousCameraProjectionMatrix * previousCameraViewMatrix);
				m_jitterAlphaClippedEqualDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
				m_jitterAlphaClippedEqualDebug.vs.debugMode.x = debugMode;
				m_jitterAlphaClippedEqualDebug.ps.debugMode.x = debugMode;

				cmd.bindPipe(m_jitterAlphaClippedEqualDebug);
				cmd.executeIndexedIndirect(
					indexes.indexBuffer,
					draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
					draws.clusterRendererExecuteIndirectCount.buffer, 0);
			}
		}
	}
}
