#include "engine/rendering/renderers/RenderDepth.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/rendering/GBuffer.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/SamplerDescription.h"
#include "components/Camera.h"

namespace engine
{
	RenderDepth::RenderDepth(Device& device)
		: m_device{ device }
		, m_default{ device.createPipeline<shaders::RenderDepthDefault>() }
		, m_alphaClipped{ device.createPipeline<shaders::RenderDepthAlphaClipped>() }
		, m_jitter{ device.createPipeline<shaders::RenderDepthJitter>() }
		, m_jitterAlphaClipped{ device.createPipeline<shaders::RenderDepthJitterAlphaClipped>() }
		, m_defaultBias{ device.createPipeline<shaders::RenderDepthDefault>() }
		, m_alphaClippedBias{ device.createPipeline<shaders::RenderDepthAlphaClipped>() }
		, m_jitterBias{ device.createPipeline<shaders::RenderDepthJitter>() }
		, m_jitterAlphaClippedBias{ device.createPipeline<shaders::RenderDepthJitterAlphaClipped>() }
	{
		m_default.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_default.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_default.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));

		m_alphaClipped.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_alphaClipped.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
		m_alphaClipped.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_alphaClipped.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));

		m_jitter.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_jitter.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_jitter.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));

		m_jitterAlphaClipped.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_jitterAlphaClipped.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
		m_jitterAlphaClipped.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_jitterAlphaClipped.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));

		m_defaultBias.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_defaultBias.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_defaultBias.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_defaultBias.setRasterizerState(RasterizerDescription()
			.depthBias(-16384)
			.depthBiasClamp(1000000.0f)
			//.depthClipEnable(false)
			.slopeScaledDepthBias(-1.75f)
			.cullMode(CullMode::None));

		m_alphaClippedBias.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_alphaClippedBias.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
		m_alphaClippedBias.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_alphaClippedBias.setRasterizerState(RasterizerDescription()
			.depthBias(-16384)
			.depthBiasClamp(1000000.0f)
			//.depthClipEnable(false)
			.slopeScaledDepthBias(-1.75f)
			.cullMode(CullMode::None));
		m_alphaClippedBias.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));

		m_jitterBias.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_jitterBias.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_jitterBias.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_jitterBias.setRasterizerState(RasterizerDescription()
			.depthBias(-16384)
			.depthBiasClamp(1000000.0f)
			//.depthClipEnable(false)
			.slopeScaledDepthBias(-1.75f)
			.cullMode(CullMode::None));

		m_jitterAlphaClippedBias.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_jitterAlphaClippedBias.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
		m_jitterAlphaClippedBias.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_jitterAlphaClippedBias.setRasterizerState(RasterizerDescription()
			.depthBias(-16384)
			.depthBiasClamp(1000000.0f)
			//.depthClipEnable(false)
			.slopeScaledDepthBias(-1.75f)
			.cullMode(CullMode::None));
		m_jitterAlphaClippedBias.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
	}

	void RenderDepth::createIhanVitunSama(int width, int height)
	{
		if (!m_vittuihansama.resource().valid() || m_vittuihansama.resource().width() != width || m_vittuihansama.resource().height() != height)
		{
			m_vittuihansama = m_device.createTextureRTV(TextureDescription()
				.width(width)
				.height(height)
				.format(Format::R16_UINT)
				.usage(ResourceUsage::GpuRenderTargetReadWrite)
				.name("Lighting target")
				.dimension(ResourceDimension::Texture2D)
				.optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));
		}
	}

	void RenderDepth::render(
		CommandList& cmd,
		ClusterDataLine& clusters,
		IndexDataLine& indexes,
		DrawDataLine& draws,
		TextureDSV depth,
		Camera& camera,
		JitterOption jitter,
		AlphaClipOption alphaClipped,
		BiasOption bias,
		const Vector2<int>& virtualResolution)
	{
		if (jitter == JitterOption::Disabled &&
			alphaClipped == AlphaClipOption::Disabled &&
			bias == BiasOption::Disabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_default.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_default.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_default.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_default.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();

			m_default.vs.clusters = clusters.clustersSRV;
			m_default.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
			cmd.bindPipe(m_default);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if(jitter == JitterOption::Disabled &&
				alphaClipped == AlphaClipOption::Enabled &&
				bias == BiasOption::Disabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_alphaClipped.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_alphaClipped.vs.uv = m_device.modelResources().gpuBuffers().uv();
			m_alphaClipped.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_alphaClipped.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_alphaClipped.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
			m_alphaClipped.vs.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();

			m_alphaClipped.vs.clusters = clusters.clustersSRV;
			m_alphaClipped.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
			m_alphaClipped.ps.materialTextures = m_device.modelResources().textures();
			//m_alphaClipped.ps.alphaMasks = m_device.modelResources().alphaMasks();
			cmd.bindPipe(m_alphaClipped);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if(jitter == JitterOption::Enabled &&
				alphaClipped == AlphaClipOption::Disabled &&
				bias == BiasOption::Disabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_jitter.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_jitter.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_jitter.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_jitter.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();

			m_jitter.vs.clusters = clusters.clustersSRV;
			m_jitter.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
			cmd.bindPipe(m_jitter);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if(jitter == JitterOption::Enabled &&
				alphaClipped == AlphaClipOption::Enabled &&
				bias == BiasOption::Disabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_jitterAlphaClipped.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_jitterAlphaClipped.vs.uv = m_device.modelResources().gpuBuffers().uv();
			m_jitterAlphaClipped.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_jitterAlphaClipped.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_jitterAlphaClipped.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
			m_jitterAlphaClipped.vs.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();

			m_jitterAlphaClipped.vs.clusters = clusters.clustersSRV;
			m_jitterAlphaClipped.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
			m_jitterAlphaClipped.ps.materialTextures = m_device.modelResources().textures();
			//m_jitterAlphaClipped.ps.alphaMasks = m_device.modelResources().alphaMasks();
			cmd.bindPipe(m_jitterAlphaClipped);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if (jitter == JitterOption::Disabled &&
				 alphaClipped == AlphaClipOption::Disabled &&
				 bias == BiasOption::Enabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = OFF, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_defaultBias.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_defaultBias.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_defaultBias.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_defaultBias.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();

			m_defaultBias.vs.clusters = clusters.clustersSRV;
			m_defaultBias.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
			cmd.bindPipe(m_defaultBias);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if(jitter == JitterOption::Disabled &&
				alphaClipped == AlphaClipOption::Enabled &&
				bias == BiasOption::Enabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = OFF, alphaclip = ON, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = OFF, alphaclip = ON, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_alphaClippedBias.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_alphaClippedBias.vs.uv = m_device.modelResources().gpuBuffers().uv();
			m_alphaClippedBias.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_alphaClippedBias.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_alphaClippedBias.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
			m_alphaClippedBias.vs.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();

			m_alphaClippedBias.vs.clusters = clusters.clustersSRV;
			m_alphaClippedBias.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
			m_alphaClippedBias.ps.materialTextures = m_device.modelResources().textures();
			//m_alphaClippedBias.ps.alphaMasks = m_device.modelResources().alphaMasks();
			cmd.bindPipe(m_alphaClippedBias);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if(jitter == JitterOption::Enabled &&
				alphaClipped == AlphaClipOption::Disabled &&
				bias == BiasOption::Enabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = OFF, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = OFF, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_jitterBias.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_jitterBias.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_jitterBias.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_jitterBias.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();

			m_jitterBias.vs.clusters = clusters.clustersSRV;
			m_jitterBias.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
			cmd.bindPipe(m_jitterBias);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if(jitter == JitterOption::Enabled &&
				alphaClipped == AlphaClipOption::Enabled &&
				bias == BiasOption::Enabled)
		{
			CPU_MARKER(cmd.api(), "Render depth (jitter = ON, alphaclip = ON, bias = OFF)");
			GPU_MARKER(cmd, "Render depth (jitter = ON, alphaclip = ON, bias = OFF)");

			cmd.setRenderTargets({}, depth);

			m_jitterAlphaClippedBias.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
			m_jitterAlphaClippedBias.vs.uv = m_device.modelResources().gpuBuffers().uv();
			m_jitterAlphaClippedBias.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
			m_jitterAlphaClippedBias.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
			m_jitterAlphaClippedBias.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
			m_jitterAlphaClippedBias.vs.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();

			m_jitterAlphaClippedBias.vs.clusters = clusters.clustersSRV;
			m_jitterAlphaClippedBias.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());
			m_jitterAlphaClippedBias.ps.materialTextures = m_device.modelResources().textures();
			//m_jitterAlphaClippedBias.ps.alphaMasks = m_device.modelResources().alphaMasks();
			cmd.bindPipe(m_jitterAlphaClippedBias);
			cmd.executeIndexedIndirect(
				indexes.indexBuffer,
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
	}

}
