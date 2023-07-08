#include "engine/rendering/ShapeRenderer.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/rendering/DepthPyramid.h"
#include "engine/rendering/LightData.h"
#include "components/Camera.h"
#include "engine/rendering/ShapeMeshFactory.h"
#include <algorithm>

#undef HARDWARE_CONSERVATIVE_RASTERIZATION
#define MSAA_BASED_CONSERVATIVE_RASTERIZATION
#define LIGHTBINS_ZTEST

namespace engine
{
	ShapeRenderer::ShapeRenderer(Device& device)
		: m_device{ device }
		, m_clearLightBins{ device.createPipeline<shaders::ClearLightBins>() }
		, m_shapeRender{ device.createPipeline<shaders::ShapeRender>() }
		, m_renderCones{ device.createPipeline<shaders::RenderCones>() }
        , m_renderSpheres{ device.createPipeline<shaders::RenderSpheres>() }
		, m_cpusphere{ ShapeMeshFactory::createSphere(Vector3f{ 0.0f, 0.0f, 0.0f }, 1.0f, 30, 30) }
		, m_cpucone{ ShapeMeshFactory::createSpot(Vector3f{ 0.0f, 0.0f, 0.0f }, Vector3f{}, 10.0f, 33, SpotSectors) }
		, m_sphere{ device, m_cpusphere }
		, m_cone{ device, m_cpucone }
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

		m_shapeRender.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_shapeRender.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
		m_shapeRender.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::Zero)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

#ifdef LIGHTBINS_ZTEST
		CullMode cullMode = CullMode::None;
#else
		CullMode cullMode = CullMode::Back;
#endif

		m_renderCones.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
#if !defined(HARDWARE_CONSERVATIVE_RASTERIZATION) && !defined(MSAA_BASED_CONSERVATIVE_RASTERIZATION)
		m_renderCones.setRasterizerState(RasterizerDescription().cullMode(cullMode));
#endif
#ifdef HARDWARE_CONSERVATIVE_RASTERIZATION
		m_renderCones.setRasterizerState(RasterizerDescription().cullMode(cullMode).conservativeRaster(ConservativeRasterizationMode::On));
#endif
#ifdef MSAA_BASED_CONSERVATIVE_RASTERIZATION
		m_renderCones.setRasterizerState(RasterizerDescription().cullMode(cullMode).multisampleEnable(true).frontCounterClockwise(true));
#endif
		m_renderCones.setDepthStencilState(DepthStencilDescription()
#ifdef LIGHTBINS_ZTEST
			.depthEnable(true)
#else
            .depthEnable(false)
#endif
			.depthWriteMask(DepthWriteMask::Zero)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

        m_renderSpheres.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
#if !defined(HARDWARE_CONSERVATIVE_RASTERIZATION) && !defined(MSAA_BASED_CONSERVATIVE_RASTERIZATION)
        m_renderSpheres.setRasterizerState(RasterizerDescription().cullMode(cullMode));
#endif
#ifdef HARDWARE_CONSERVATIVE_RASTERIZATION
        m_renderSpheres.setRasterizerState(RasterizerDescription().cullMode(cullMode).conservativeRaster(ConservativeRasterizationMode::On));
#endif
#ifdef MSAA_BASED_CONSERVATIVE_RASTERIZATION
        m_renderSpheres.setRasterizerState(RasterizerDescription().cullMode(cullMode).multisampleEnable(true));
#endif
        m_renderSpheres.setDepthStencilState(DepthStencilDescription()
#ifdef LIGHTBINS_ZTEST
            .depthEnable(true)
#else
            .depthEnable(false)
#endif
            .depthWriteMask(DepthWriteMask::Zero)
            .depthFunc(ComparisonFunction::GreaterEqual)
            .frontFace(front)
            .backFace(back));
	}

	void ShapeRenderer::clearBins(CommandList& cmd, const Camera& camera)
	{
		m_clearLightBins.cs.lightBins = m_lightBins;
		m_clearLightBins.cs.binCount.x = binSize(camera.width()) * binSize(camera.height());
		m_clearLightBins.cs.maxLightsPerBin.x = MaxLightsPerBin;
		cmd.bindPipe(m_clearLightBins);
		cmd.dispatch(roundUpToMultiple(m_clearLightBins.cs.binCount.x, 64u) / 64u, 1, 1);
	}

	void ShapeRenderer::createPointlightBinTexture(size_t width, size_t height)
	{
		if (!m_pointlightBinTexture.resource().valid() || m_pointlightBinTexture.resource().width() != width || m_pointlightBinTexture.resource().height() != height)
		{
			m_pointlightBinTexture = m_device.createTextureRTV(TextureDescription()
				.width(width)
				.height(height)
				.format(Format::R16_UINT)
				.usage(ResourceUsage::GpuRenderTargetReadWrite)
				.name("Lighting target")
				.dimension(ResourceDimension::Texture2D)
				.optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));
		}
	}

	void ShapeRenderer::render(
		CommandList& cmd, 
		DepthPyramid& /*depthPyramid*/,
		const Camera& camera,
		const Matrix4f& jitterViewProjection,
		LightData& lights)
	{
		if (createLightBins(camera.width(), camera.height()))
		{
			cmd.clearBuffer(m_lightBins.resource());
		}

		{
			CPU_MARKER(cmd.api(), "Clear Light bins");
			GPU_MARKER(cmd, "Clear Light bins");
			clearBins(cmd, camera);
		}

#ifdef LIGHTBINS_ZTEST
		{
			CPU_MARKER(cmd.api(), "Copy depth for light bins");
			GPU_MARKER(cmd, "Copy depth for light bins");
			cmd.clearDepthStencilView(m_shapeDSVView, 0.0f);
			//cmd.copyTexture(depthPyramid.srvs()[3], m_shapeDSVView);
		}
#endif

        {
            CPU_MARKER(cmd.api(), "Render Pointlight bins");
            GPU_MARKER(cmd, "Render Pointlight bins");

			createPointlightBinTexture(m_shapeDSVView.width(), m_shapeDSVView.height());

#ifdef LIGHTBINS_ZTEST
            cmd.setRenderTargets({ m_pointlightBinTexture }, m_shapeDSVView);
#else
            cmd.setRenderTargets({ m_shapeRTVView });
#endif

            m_renderSpheres.vs.jitterViewProjectionMatrix = fromMatrix(
                jitterViewProjection * camera.projectionMatrix() * camera.viewMatrix());
            m_renderSpheres.vs.vertexes = m_sphere.view_vertices;
            m_renderSpheres.ps.lightBins = m_lightBins;
            m_renderSpheres.ps.binSize.x = binSize(camera.width());
            m_renderSpheres.ps.binSize.y = binSize(camera.height());
            m_renderSpheres.ps.pitch.x = m_renderSpheres.ps.binSize.x * MaxLightsPerBin;
            m_renderSpheres.ps.maxLightsPerBin.x = MaxLightsPerBin;

            if (lights.count() > 0)
            {
                m_renderSpheres.vs.lightParameters = lights.parameters();
                m_renderSpheres.vs.pointLightRange = lights.pointRanges();
                m_renderSpheres.vs.pointLightIds = lights.pointIds();
                m_renderSpheres.vs.lightTransforms = lights.transforms();
            }

            m_renderSpheres.vs.sectors.x = SpotSectors;

            cmd.bindPipe(m_renderSpheres);
            cmd.drawIndexedInstanced(
                m_sphere.view_indexes,
                m_sphere.view_indexes.desc().elements,
                static_cast<uint32_t>(lights.cputransforms().size()),
                0, 0, 0);
        }

		{
			CPU_MARKER(cmd.api(), "Render Spotlight bins");
			GPU_MARKER(cmd, "Render Spotlight bins");

			createPointlightBinTexture(m_shapeDSVView.width(), m_shapeDSVView.height());

#ifdef LIGHTBINS_ZTEST
			cmd.setRenderTargets({ m_pointlightBinTexture }, m_shapeDSVView);
#else
			cmd.setRenderTargets({ m_shapeRTVView });
#endif

			m_renderCones.vs.jitterViewProjectionMatrix = fromMatrix(
				jitterViewProjection * camera.projectionMatrix() * camera.viewMatrix());
			m_renderCones.vs.cameraPosition = camera.position();
			m_renderCones.ps.lightBins = m_lightBins;
			m_renderCones.ps.binSize.x = binSize(camera.width());
			m_renderCones.ps.binSize.y = binSize(camera.height());
			m_renderCones.ps.pitch.x = m_renderCones.ps.binSize.x * MaxLightsPerBin;
			m_renderCones.ps.maxLightsPerBin.x = MaxLightsPerBin;

			if (lights.count() > 0)
			{
				m_renderCones.vs.lightParameters = lights.parameters();
				m_renderCones.vs.spotLightRange = lights.spotRanges();
				m_renderCones.vs.spotLightIds = lights.spotIds();
				m_renderCones.vs.spotLightTransforms = lights.spotTransforms();
				m_renderCones.vs.lightPositions = lights.worldPositions();
				m_renderCones.vs.lightDirections = lights.directions();
			}
			else
			{
				// TODO: fix vulkan pipeline to know the difference between structured and typed buffers
				if (!m_lightParameters.resource().valid())
				{
					engine::vector<Vector4f> parameters;
					parameters.emplace_back(Vector4f{ 0.0f, 0.0f, 0.0f, 0.0f });
					m_lightParameters = m_device.createBufferSRV(BufferDescription()
						.name("lightParameters")
						.format(Format::R32G32B32A32_FLOAT)
						.setInitialData(BufferDescription::InitialData(parameters)));
				}

				if (!m_spotLightRanges.resource().valid())
				{
					engine::vector<float> spotranges;
					spotranges.emplace_back(0.0f);
					m_spotLightRanges = m_device.createBufferSRV(BufferDescription()
						.name("spotLightRanges")
						.format(Format::R32_FLOAT)
						.setInitialData(BufferDescription::InitialData(spotranges)));
				}

				if (!m_spotLightIds.resource().valid())
				{
					engine::vector<uint32_t> spotids;
					spotids.emplace_back(0u);
					m_spotLightIds = m_device.createBufferSRV(BufferDescription()
						.name("spotLightIds")
						.format(Format::R32_UINT)
						.setInitialData(BufferDescription::InitialData(spotids)));
				}

				if (!m_spotLightTransforms.resource().valid())
				{
					engine::vector<Matrix4f> spottransforms;
					spottransforms.emplace_back(Matrix4f::identity());
					m_spotLightTransforms = m_device.createBufferSRV(BufferDescription()
						.name("spotTransforms")
						.usage(ResourceUsage::GpuReadWrite)
						.structured(true)
						.elements(spottransforms.size())
						.elementSize(sizeof(float4x4))
						.setInitialData(BufferDescription::InitialData(spottransforms)));
				}

				m_renderCones.vs.lightParameters = m_lightParameters;
				m_renderCones.vs.spotLightRange = m_spotLightRanges;
				m_renderCones.vs.spotLightIds = m_spotLightIds;
				m_renderCones.vs.spotLightTransforms = m_spotLightTransforms;
			}
			m_renderCones.vs.sectors.x = SpotSectors;

			cmd.bindPipe(m_renderCones);
			cmd.drawIndexedInstanced(
				m_cone.view_indexes,
				m_cone.view_indexes.desc().elements,
				static_cast<uint32_t>(lights.cpuspotranges().size()),
				0, 0, 0);
		}

        
	}

	bool ShapeRenderer::createLightBins(int width, int height)
	{
		bool res = false;
		auto elements = binSize(width) * binSize(height) * MaxLightsPerBin;
		if (!m_lightBins || m_lightBins.resource().desc().elements != elements)
		{
			m_lightBins = m_device.createBufferUAV(BufferDescription()
				.name("LightBins")
				.elements(elements)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite));
			m_lightBinsSRV = m_device.createBufferSRV(m_lightBins);
			m_lightBinsSRVView = m_lightBinsSRV.resource();

#ifdef LIGHTBINS_ZTEST
			m_shapeDSV = m_device.createTextureDSV(TextureDescription()
				.width(binSize(width))
				.height(binSize(height))
				.format(Format::D32_FLOAT)
				.usage(ResourceUsage::DepthStencil)
				.name("Shape render dsv")
				.dimension(ResourceDimension::Texture2D)
				.optimizedDepthClearValue(0.0f));

			m_shapeDSVView = m_shapeDSV.resource();
#endif

			m_shapeRTV = m_device.createTextureRTV(TextureDescription()
				.width(binSize(width))
				.height(binSize(height))
#ifdef MSAA_BASED_CONSERVATIVE_RASTERIZATION
				.samples(8)
#endif
				.format(Format::R8_UINT)
				.usage(ResourceUsage::GpuRenderTargetReadWrite)
				.name("Shape render rtv")
				.dimension(ResourceDimension::Texture2D)
				.optimizedDepthClearValue(0.0f));

			m_shapeRTVView = m_shapeRTV.resource();

			res = true;
		}
		return res;
	}

	BufferSRV ShapeRenderer::lightBins()
	{
		return m_lightBinsSRVView;
	}
}
