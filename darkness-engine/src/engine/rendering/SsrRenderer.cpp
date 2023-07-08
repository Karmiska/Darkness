#include "engine/rendering/SsrRenderer.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"
#include "engine/rendering/DepthPyramid.h"
#include "engine/rendering/GBuffer.h"
#include "components/Camera.h"
#include "engine/rendering/tools/BlueNoiseCPU.h"
#include "tools/image/Image.h"
#include "tools/PathTools.h"
#include "platform/Environment.h"
#include <algorithm>

namespace engine
{
	#include "shaders/core/shared_types/SSRDebug.hlsli"

    SsrRenderer::SsrRenderer(Device& device)
        : m_device{ device }
        , m_pipeline{ device.createPipeline<shaders::SSR>() }
		, m_pipelineForward{ device.createPipeline<shaders::SSRForward>() }
		, m_roughnessPipeline{ device.createPipeline<shaders::GBufferRoughness>() }
		//, m_blur{ device.createPipeline<shaders::Blur>(shaderStorage) }
		, m_mousexSSRDebug{ device.width() / 2 }
		, m_mouseySSRDebug{ device.height() / 2 }
    {
		m_pipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
		m_pipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
		m_pipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
		//m_pipeline.ps.depthSampler = device.createSampler(SamplerDescription().filter(Filter::Point));

		m_pipelineForward.ps.depthSampler = device.createSampler(SamplerDescription().filter(Filter::Point));
		m_pipelineForward.ps.colorSampler = device.createSampler(SamplerDescription().filter(Filter::Point));
		m_pipelineForward.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
		m_pipelineForward.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
		m_pipelineForward.setDepthStencilState(DepthStencilDescription().depthEnable(false));

		m_roughnessPipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
		m_roughnessPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
		m_roughnessPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
		m_roughnessPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));

		
	}

	void SsrRenderer::createBuffers()
	{
		engine::string path = pathClean(pathJoin(getExecutableDirectory(), engine::string("..\\..\\..\\..\\..\\darkness-engine\\data\\bluenoise_4ch_hdr.png")));
		auto blueNoiseImage = image::Image::createImage(path, image::ImageType::EXTERNAL);
		auto key = tools::hash(pathClean(path));
		m_noiseTexture = m_device.createTextureSRV(key, TextureDescription()
			.name("color")
			.width(static_cast<uint32_t>(blueNoiseImage->width()))
			.height(static_cast<uint32_t>(blueNoiseImage->height()))
			.format(blueNoiseImage->format())
			.arraySlices(static_cast<uint32_t>(blueNoiseImage->arraySlices()))
			.mipLevels(static_cast<uint32_t>(blueNoiseImage->mipCount()))
			.setInitialData(TextureDescription::InitialData(
				tools::ByteRange(
					blueNoiseImage->data(),
					blueNoiseImage->data() + blueNoiseImage->bytes()),
				static_cast<uint32_t>(blueNoiseImage->width()), static_cast<uint32_t>(blueNoiseImage->width() * blueNoiseImage->height()))));

		m_ssrDebugUAV = m_device.createBufferUAV(BufferDescription()
			.elementSize(sizeof(SSRDebug))
			.elements(100000)
			.usage(ResourceUsage::GpuReadWrite)
			.structured(true)
			.name("SSR Debug buffer"));
		m_ssrDebugSRV = m_device.createBufferSRV(m_ssrDebugUAV);

		m_ssrDebugCounterUAV = m_device.createBufferUAV(BufferDescription()
			.elementSize(sizeof(uint32_t))
			.elements(1)
			.format(Format::R32_UINT)
			.usage(ResourceUsage::GpuRead)
			.name("Count buffer"));
		m_ssrDebugCounterSRV = m_device.createBufferSRV(m_ssrDebugCounterUAV);

		/*m_noiseTexture = device.createTextureSRV(TextureDescription()
			.width(64)
			.height(64)
			.format(Format::R32G32_FLOAT)
			.name("SSR noise texture")
			.setInitialData(
				TextureDescription::InitialData(
				tools::ByteRange(
					reinterpret_cast<uint8_t*>(&BlueNoise4096[0]),
					reinterpret_cast<uint8_t*>(&BlueNoise4096[0])+(sizeof(Float2) * 4096)),
				sizeof(Float2) * 64,
				sizeof(Float2) * 4096)));*/
	}

	float2 tileCount(float level, float2 frameSize)
	{
		return level == 0 ? frameSize : frameSize / std::exp2(level);
	}

	float2 tileSize(float /*level*/, float2 tileCount, float2 frameSize)
	{
		return frameSize / tileCount;
	}

	float2 tilePosition(float2 pos, float2 tileSize)
	{
		return pos / tileSize;
	}

	float2 floor(float2 val)
	{
		return { std::floorf(val.x), std::floorf(val.y) };
	}

	bool linePlaneIntersect(float2& intersection, float2 linePoint, float2 lineVec, float2 planeNormal, float2 planePoint)
	{
		/*intersection = float2(0.0f, 0.0f);
		float nl = planeNormal.dot(lineVec);
		if (nl == 0)
		{
			return false;
		}

		float t = (planeNormal.dot(planePoint) - planeNormal.dot(linePoint)) / nl;
		intersection = linePoint + (lineVec * t);
		return true;*/

		/*float len;

		//calculate the distance between the linePoint and the line-plane intersection point
		float dotNumerator = (planePoint - linePoint).dot(planeNormal);
		float dotDenominator = lineVec.dot(planeNormal);

		float2 vec = float2(0.0f, 0.0f);
		intersection = float2(0.0f, 0.0f);

		//line and plane are not parallel
		if (dotDenominator != 0.0f)
		{
			len = dotNumerator / dotDenominator;

			//create a vector from the linePoint to the intersection point
			vec = (lineVec - len).magnitude();

			//get the coordinates of the line-plane intersection point
			intersection = linePoint + vec;

			return true;
		}

		//output not valid
		else {
			return false;
		}*/

		float2 diff = linePoint - planePoint;
		float prod1 = diff.dot(planeNormal);
		float prod2 = lineVec.dot(planeNormal);
		float prod3 = prod1 / (prod2 + 0.000001f);
		intersection = linePoint - lineVec * prod3;
		return true;

	}

	float2 intersectTile(float2 screenPos, float2 direction, int level, float2 frameSize)
	{
		/*float2 tSize = tileSize(level, tileCount(level));
		float2 tpos = tilePosition(screenPos, tSize);
		uint2 utpos = uint2((uint)tpos.x, (uint)tpos.y);
		float2 tileFrac = tpos - (utpos * tSize);*/

		//return screenPos + (direction * 4);
		direction = direction.normalize();

		float2 tSize = tileSize(static_cast<float>(level), tileCount(static_cast<float>(level), frameSize), frameSize);
		float2 tPos = floor(tilePosition(screenPos, tSize));

		float2 xintersect;
		float2 yintersect;
		float2 boundary;
		boundary.x = direction.x >= 0 ? (tPos.x + 1) * tSize.x : tPos.x * tSize.x;
		boundary.y = direction.y >= 0 ? (tPos.y + 1) * tSize.y : tPos.y * tSize.y;

		linePlaneIntersect(
			xintersect,
			screenPos,
			direction,
			float2(1.0f, 0.0f),
			float2(boundary.x, 0.0f));

		linePlaneIntersect(
			yintersect,
			screenPos,
			direction,
			float2(0.0f, 1.0f),
			float2(0.0f, boundary.y));

		float distx = (xintersect - screenPos).magnitude();
		float disty = (yintersect - screenPos).magnitude();

		if (distx < disty)
		{
			return xintersect;
		}
		else
		{
			return yintersect;
		}


		/*float2 cross_step = float2(direction.x >= 0.0 ? 1.0 : -1.0, direction.y >= 0.0 ? 1.0 : -1.0);
		float2 tSize = tileSize(level, tileCount(level));
		float2 cell_count = tileCount(level);
		float2 cell_id = tilePosition(screenPos, tSize);
		float2 cell_size = 1.0 / cell_count;
		float2 planes = cell_id / cell_count + cell_size * cross_step;

		float2 solutions = (planes - screenPos) / direction;
		float2 intersection_pos = screenPos + direction * min(solutions.x, solutions.y);

		float2 cross_offset = cross_step * 0.00001;
		intersection_pos += (solutions.x < solutions.y) ? float2(cross_offset.x, 0.0) : float2(0.0, cross_offset.y);

		return intersection_pos;*/
	}

	void SsrRenderer::test(float2 frameSize)
	{
		auto res = intersectTile({ 1.0f, 1.0f }, { 1.0, 0.0 }, 0, frameSize);
	}

    void SsrRenderer::render(
        Device& device,
        CommandList& cmd,
		DepthPyramid& depthPyramid,
		TextureSRV frame,
		GBuffer& gbuffer,
        Camera& camera)
    {
		if (!m_noiseTexture)
			createBuffers();
		/*test({ static_cast<float>(device.width()), 
			static_cast<float>(device.height())});*/

		test({ 685.000000, 549.000000 });

		CPU_MARKER(cmd.api(), "SSR");
		GPU_MARKER(cmd, "SSR");

		{
			CPU_MARKER(cmd.api(), "SSR Debug clean");
			GPU_MARKER(cmd, "SSR Debug clean");

			cmd.clearBuffer(m_ssrDebugCounterUAV);
		}

		{
			if (!m_rtv ||
				m_rtv.resource().width() != std::max(frame.width() >> 1ull, 1ull) ||
				m_rtv.resource().height() != std::max(frame.height() >> 1ull, 1ull))
			{
				resizeTarget(cmd, frame.width(), frame.height());
			}
		}

		{
			CPU_MARKER(cmd.api(), "Render roughness for SSR");
			GPU_MARKER(cmd, "Render roughness for SSR");
			cmd.setRenderTargets({ m_roughnessRTV });

			m_roughnessPipeline.ps.gbufferInstanceId = gbuffer.srv(GBufferType::InstanceId);
			m_roughnessPipeline.ps.gbufferUV = gbuffer.srv(GBufferType::Uv);
			m_roughnessPipeline.ps.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();
			m_roughnessPipeline.ps.materialTextures = m_device.modelResources().textures();

			cmd.bindPipe(m_roughnessPipeline);
			// TODO: should be 3 right?
			cmd.draw(4u);
		}

		{
			CPU_MARKER(cmd.api(), "Clear SSR RTV");
			GPU_MARKER(cmd, "Clear SSR RTV");
			cmd.clearRenderTargetView(m_rtv);
		}

		{
			CPU_MARKER(cmd.api(), "Render SSR");
			GPU_MARKER(cmd, "Render SSR");

			cmd.setRenderTargets({ m_rtv });

			m_pipeline.ps.color = frame;
			m_pipeline.ps.depthPyramid = depthPyramid.depthMax();
			m_pipeline.ps.rougnessMetalness = m_roughnessSRV;
			m_pipeline.ps.gbufferNormals = gbuffer.srv(GBufferType::Normal);
			m_pipeline.ps.gbufferMotion = gbuffer.srv(GBufferType::Motion);
			m_pipeline.ps.noiseTexture = m_noiseTexture;

			m_pipeline.ps.ssrDebugBuffer = m_ssrDebugUAV;
			m_pipeline.ps.ssrDebugBufferCounter = m_ssrDebugCounterUAV;
			m_pipeline.ps.ssrDebugPoint = { static_cast<uint32_t>(m_mousexSSRDebug / 2), static_cast<uint32_t>(m_mouseySSRDebug / 2) };

			//m_pipeline.ps.random = m_distribution(m_generator);
			m_pipeline.ps.frameNumber.x = static_cast<uint32_t>(device.frameNumber());
			m_pipeline.ps.jitter = camera.jitterValue(m_pipeline.ps.frameNumber.x);


			m_pipeline.ps.cameraProjectionMatrix = fromMatrix(camera.projectionMatrix(Vector2<int>{
				static_cast<int>(frame.width()), 
				static_cast<int>(frame.height()) }));
			m_pipeline.ps.cameraViewMatrix = fromMatrix(camera.viewMatrix());

			m_pipeline.ps.invView = fromMatrix(camera.viewMatrix().inverse());
			m_pipeline.ps.invProjection = fromMatrix(camera.projectionMatrix(Vector2<int>{ 
				static_cast<int>(frame.width()), 
				static_cast<int>(frame.height()) }).inverse());
			m_pipeline.ps.cameraWorldSpacePosition = camera.position();
			m_pipeline.ps.cameraForward = camera.forward();
			m_pipeline.ps.cameraUp = camera.up();
			m_pipeline.ps.cameraRight = camera.right();
			m_pipeline.ps.fov = camera.fieldOfView();

			m_pipeline.ps.nearFar = Float2{ camera.nearPlane(), camera.farPlane() };
			m_pipeline.ps.frameSize = Float2{ 
				static_cast<float>(frame.width()), 
				static_cast<float>(frame.height()) };
            m_pipeline.ps.pow2size = Float2{ 
                static_cast<float>(roundUpToPow2(static_cast<int>(frame.width()))), 
                static_cast<float>(roundUpToPow2(static_cast<int>(frame.height()))) };

			//m_pipeline.ps.pow2size = Vector2f{ static_cast<float>(roundUpToPow2(frame.width())), static_cast<float>(roundUpToPow2(frame.height())) };

			m_pipeline.ps.onePerFrameSize = Float2{ 
				1.0f / static_cast<float>(frame.width()), 
				1.0f / static_cast<float>(frame.height()) };
			m_pipeline.ps.mipLevels = static_cast<float>(depthPyramid.depth().texture().mipLevels());

			cmd.bindPipe(m_pipeline);
			// TODO: should be 3 right?
			cmd.draw(4u);
		}

		/*{
			CPU_MARKER(cmd.api(), "Blur SSR");
			GPU_MARKER(cmd, "Blur SSR");

			m_blur.cs.input = m_srv;
			m_blur.cs.output = m_blurUav;

			cmd.bindPipe(m_blur);
			cmd.dispatch(
				std::max(roundUpToMultiple(frame.width(), 8u) / 8u, 1u),
				std::max(roundUpToMultiple(frame.height(), 8u) / 8u, 1u),
				1);
		}
		{
			CPU_MARKER(cmd.api(), "Blur SSR");
			GPU_MARKER(cmd, "Blur SSR");

			m_blur.cs.input = m_blurSrv;
			m_blur.cs.output = m_uav;

			cmd.bindPipe(m_blur);
			cmd.dispatch(
				std::max(roundUpToMultiple(frame.width(), 8u) / 8u, 1u),
				std::max(roundUpToMultiple(frame.height(), 8u) / 8u, 1u),
				1);
		}
		{
			CPU_MARKER(cmd.api(), "Blur SSR");
			GPU_MARKER(cmd, "Blur SSR");

			m_blur.cs.input = m_srv;
			m_blur.cs.output = m_blurUav;

			cmd.bindPipe(m_blur);
			cmd.dispatch(
				std::max(roundUpToMultiple(frame.width(), 8u) / 8u, 1u),
				std::max(roundUpToMultiple(frame.height(), 8u) / 8u, 1u),
				1);
		}*/
    }

	void SsrRenderer::renderForward(
		Device& /*device*/,
		CommandList& cmd,
		TextureSRV depthView,
		TextureSRV frame,
		Camera& camera)
	{
		{
			if (!m_rtv ||
				m_rtv.resource().width() != std::max(frame.width() >> 1ull, 1ull) ||
				m_rtv.resource().height() != std::max(frame.height() >> 1ull, 1ull))
			{
				resizeTarget(cmd, frame.width(), frame.height());
			}
		}

		{
			CPU_MARKER(cmd.api(), "Render SSR");
			GPU_MARKER(cmd, "Render SSR");

			cmd.setRenderTargets({ m_rtv });

			m_pipelineForward.ps.color = frame;
			m_pipelineForward.ps.depthTexture = depthView;

			m_pipelineForward.ps.cameraProjectionMatrix = fromMatrix(camera.projectionMatrix(Vector2<int>{ static_cast<int>(frame.width()), static_cast<int>(frame.height()) }));
			m_pipelineForward.ps.cameraViewMatrix = fromMatrix(camera.viewMatrix());

			m_pipelineForward.ps.invView = fromMatrix(camera.viewMatrix().inverse());
			m_pipelineForward.ps.invProjection = fromMatrix(camera.projectionMatrix(Vector2<int>{ static_cast<int>(frame.width()), static_cast<int>(frame.height()) }).inverse());
			m_pipelineForward.ps.cameraWorldSpacePosition = camera.position();

			m_pipelineForward.ps.nearFar = Float2{ camera.nearPlane(), camera.farPlane() };
			m_pipelineForward.ps.frameSize = Float2{ static_cast<float>(frame.width()), static_cast<float>(frame.height()) };
			m_pipelineForward.ps.onePerFrameSize = Float2{ 1.0f / static_cast<float>(frame.width()), 1.0f / static_cast<float>(frame.height()) };

			cmd.bindPipe(m_pipelineForward);
			// TODO: should be 3 right?
			cmd.draw(4u);
		}
	}

	void SsrRenderer::resizeTarget(CommandList& cmd, size_t width, size_t height)
	{
		CPU_MARKER(cmd.api(), "Render SSR");
		GPU_MARKER(cmd, "Render SSR");

		m_rtv = m_device.createTextureRTV(TextureDescription()
			.format(Format::R16G16B16A16_FLOAT)
			.width(std::max(width >> 1ull, 1ull))
			.height(std::max(height >> 1ull, 1ull))
			.mipLevels(1)
			.usage(ResourceUsage::GpuRenderTargetReadWrite)
			.name("SSR Render target"));
		//m_uav = m_device.createTextureUAV(m_rtv);
		m_srv = m_device.createTextureSRV(m_rtv);

		/*m_blurUav = m_device.createTextureUAV(TextureDescription()
			.format(Format::R16G16B16A16_FLOAT)
			.width(width)
			.height(height)
			.mipLevels(1)
			.usage(ResourceUsage::GpuRenderTargetReadWrite)
			.name("SSR Blur target"));
		m_blurSrv = m_device.createTextureSRV(m_blurUav);*/

		m_roughnessRTV = m_device.createTextureRTV(TextureDescription()
			.format(Format::R8G8_UNORM)
			.width(std::max(width >> 1ull, 1ull))
			.height(std::max(height >> 1ull, 1ull))
			.mipLevels(1)
			.usage(ResourceUsage::GpuRenderTargetReadWrite)
			.name("SSR Roughness target"));
		m_roughnessSRV = m_device.createTextureSRV(m_roughnessRTV);
	}

	TextureSRV SsrRenderer::ssr()
	{
		return m_srv;
	}

	BufferSRV SsrRenderer::ssrDebug()
	{
		return m_ssrDebugSRV;
	}

	BufferSRV SsrRenderer::ssrDebugCount()
	{
		return m_ssrDebugCounterSRV;
	}

	void SsrRenderer::setSSRDebugMousePosition(int x, int y)
	{
		m_mousexSSRDebug = x;
		m_mouseySSRDebug = y;
	}
}
