#include "engine/rendering/ClusterRendererTransparent.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Common.h"
#include "engine/rendering/LightData.h"
#include "engine/rendering/ShapeRenderer.h"
#include "shaders/core/shared_types/DebugModes.hlsli"
#include "components/Camera.h"
#include <algorithm>

namespace engine
{
    ClusterRendererTransparent::ClusterRendererTransparent(Device& device)
        : m_device{ device }
        , m_pipelines{}
		, m_blurDownsample{ device.createPipeline<shaders::BlurDownsample>() }
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

        // lighting
        for (int i = 0; i < 2; ++i)
        {
            auto transparentPipe = device.createPipeline<shaders::ClusterRenderTransparent>();
            transparentPipe.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
            transparentPipe.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
            transparentPipe.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
            transparentPipe.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
            transparentPipe.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
            transparentPipe.ps.shadow_sampler = device.createSampler(SamplerDescription()
                .addressU(TextureAddressMode::Mirror)
                .addressV(TextureAddressMode::Mirror)
                .filter(Filter::Comparison));
            transparentPipe.ps.debug = i != 0;
            transparentPipe.setDepthStencilState(DepthStencilDescription()
                .depthEnable(true)
                .depthWriteMask(DepthWriteMask::All)
                .depthFunc(ComparisonFunction::GreaterEqual)
                .frontFace(front)
                .backFace(back));
            m_pipelines.emplace_back(std::move(transparentPipe));
        }
    }

    void ClusterRendererTransparent::createDownsamples(CommandList& cmd, TextureSRV frame)
    {
        CPU_MARKER(cmd.api(), "Downsample frame");
        GPU_MARKER(cmd, "Downsample frame");

		if (!m_frameBlurRTV ||
			(frame.width() >> 1) != m_frameBlurRTV.resource().width() ||
			(frame.height() >> 1) != m_frameBlurRTV.resource().height())
		{
			m_frameBlurRTV = m_device.createTextureRTV(TextureDescription()
				.width(std::max(frame.width() >> 1ull, 1ull))
				.height(std::max(frame.height() >> 1ull, 1ull))
				.format(frame.format())
				.usage(ResourceUsage::GpuRenderTargetReadWrite)
				.name("Transparency blur target")
				.dimension(ResourceDimension::Texture2D)
				.mipLevels(std::max((mipCount(frame.width() >> 1ull, frame.height() >> 1ull)) - 2ull, 1ull))
				.optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));
			m_frameBlurSRV = m_device.createTextureSRV(m_frameBlurRTV);

			m_frameBlurMipsUAV.resize(m_frameBlurRTV.resource().texture().mipLevels());
			m_frameBlurMipsSRV.resize(m_frameBlurRTV.resource().texture().mipLevels());
			for (uint32_t i = 0; i < m_frameBlurMipsUAV.size(); ++i)
			{
				m_frameBlurMipsUAV[i] = m_device.createTextureUAV(m_frameBlurRTV, SubResource{ i, 1, 0, 1 });
				m_frameBlurMipsSRV[i] = m_device.createTextureSRV(m_frameBlurRTV, SubResource{ i, 1, 0, 1 });
			}
		}

		// first level
		size_t width = std::max(frame.width() >> 1ull, 1ull);
		size_t height = std::max(frame.height() >> 1ull, 1ull);
		{
			m_blurDownsample.cs.input = frame;
			m_blurDownsample.cs.output = m_frameBlurMipsUAV[0];

			cmd.bindPipe(m_blurDownsample);
			cmd.dispatch(
				std::max(roundUpToMultiple(width, 8ull) / 8ull, 1ull),
				std::max(roundUpToMultiple(height, 8ull) / 8ull, 1ull),
				1ull);
		}

		// the rest
		{
			for (uint32_t i = 0; i < m_frameBlurRTV.resource().texture().mipLevels() - 1; ++i)
			{
				m_blurDownsample.cs.input = m_frameBlurMipsSRV[i];
				m_blurDownsample.cs.output = m_frameBlurMipsUAV[i + 1];

				width = std::max(width >> 1ull, 1ull);
				height = std::max(height >> 1ull, 1ull);

				cmd.bindPipe(m_blurDownsample);
				cmd.dispatch(
					std::max(roundUpToMultiple(width, 8ull) / 8ull, 1ull),
					std::max(roundUpToMultiple(height, 8ull) / 8ull, 1ull),
					1ull);
			}
		}

		m_frame = frame;
    }

    void ClusterRendererTransparent::render(
        ClusterRendererTransparencyArgs args,
        ClusterDataLine& clusters,
        IndexDataLine& indexes,
        DrawDataLine& draws,
		BufferSRV lightBins,
        TextureSRV frame)
    {
		CPU_MARKER(args.cmd->api(), "Transparency gbuffer pass");
		GPU_MARKER(*args.cmd, "Transparency gbuffer pass");

        args.cmd->setRenderTargets({ args.target }, args.depthBuffer);

        auto currentRenderMode = args.currentRenderMode == 0 ? 0 : 1;
        engine::Pipeline<shaders::ClusterRenderTransparent>& pipeline = m_pipelines[currentRenderMode];


		pipeline.vs.vertices = m_device.modelResources().gpuBuffers().vertex();
		pipeline.vs.normals = m_device.modelResources().gpuBuffers().normal();
		pipeline.vs.tangents = m_device.modelResources().gpuBuffers().tangent();
		pipeline.vs.uv = m_device.modelResources().gpuBuffers().uv();
		pipeline.vs.clusterData = m_device.modelResources().gpuBuffers().clusterBinding();
		pipeline.vs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
		pipeline.vs.scales = m_device.modelResources().gpuBuffers().instanceScale();
		pipeline.ps.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();

        pipeline.vs.clusters = clusters.clustersSRV;
        pipeline.vs.viewProjectionMatrix = fromMatrix(args.camera->projectionMatrix() * args.camera->viewMatrix());
        pipeline.vs.previousViewProjectionMatrix = fromMatrix(args.previousProjectionMatrix * args.previousViewMatrix);
        pipeline.vs.jitterViewProjectionMatrix = fromMatrix(args.camera->jitterMatrix(args.frameNumber, args.virtualResolution) * args.camera->projectionMatrix() * args.camera->viewMatrix());
        pipeline.vs.cameraPosition = args.camera->position();

        pipeline.ps.debugMode.x = args.currentRenderMode;

        if (args.currentRenderMode == DEBUG_MODE_CLUSTERS)
        {
            pipeline.vs.debug = true;
            pipeline.ps.debug = true;
        }
        else
        {
            pipeline.vs.debug = false;
            pipeline.ps.debug = false;
        }

        pipeline.ps.cameraWorldSpacePosition = args.camera->position();
        pipeline.ps.exposure = args.camera->exposure();


        pipeline.ps.frameBlurChain = m_frameBlurSRV;
		pipeline.ps.frameBlurChainMipZero = frame;
        pipeline.ps.resolution = { 
            static_cast<float>(args.camera->width()), 
            static_cast<float>(args.camera->height()),
            0.0f, 0.0f };

        
        pipeline.ps.environmentStrength = args.camera->environmentMapStrength();
        if (args.camera->environmentIrradiance().valid() && args.camera->environmentIrradiance().texture().arraySlices() == 1)
        {
            pipeline.ps.environmentIrradiance = args.camera->environmentIrradiance();
            pipeline.ps.environmentIrradianceCubemap = TextureSRV();
            pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(false);
            pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(true);
        }
        else
        {
            pipeline.ps.environmentIrradiance = TextureSRV();
            pipeline.ps.environmentIrradianceCubemap = args.camera->environmentIrradiance();
            pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(true);
            pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(false);
        }
        pipeline.ps.environmentSpecular = args.camera->environmentSpecular();
        pipeline.ps.environmentBrdfLut = args.camera->environmentBrdfLUT();
        pipeline.ps.hasEnvironmentSpecular.x = args.camera->environmentSpecular().valid();
        pipeline.ps.shadowSize = Float2{ 1.0f / static_cast<float>(ShadowMapWidth), 1.0f / static_cast<float>(ShadowMapHeight) };
        pipeline.ps.shadowMap = *args.shadowMap;
        pipeline.ps.shadowVP = *args.shadowVP;
        pipeline.ps.cameraInverseProjectionMatrix = fromMatrix(args.camera->projectionMatrix().inverse());
        pipeline.ps.cameraInverseViewMatrix = fromMatrix(args.camera->viewMatrix().inverse());

        pipeline.ps.frameBlurMipCount.x = static_cast<uint32_t>(m_frameBlurSRV.resource().texture().mipLevels());

        pipeline.ps.frameSize.x = args.camera->width();
        pipeline.ps.frameSize.y = args.camera->height();

        pipeline.ps.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();
        pipeline.ps.materialTextures = m_device.modelResources().textures();

        pipeline.ps.probePositionRange = Float4{ args.probePosition.x, args.probePosition.y, args.probePosition.z, args.probeRange };
        pipeline.ps.probeBBmin = Float4{ args.probePosition.x - args.probeRange, args.probePosition.y - args.probeRange, args.probePosition.z - args.probeRange, 0.0f };
        pipeline.ps.probeBBmax = Float4{ args.probePosition.x + args.probeRange, args.probePosition.y + args.probeRange, args.probePosition.z + args.probeRange, 0.0f };

        pipeline.ps.usingProbe.x = 0;

		pipeline.ps.binSize.x = binSize(args.camera->width());
		pipeline.ps.binSize.y = binSize(args.camera->height());

		pipeline.ps.lightBins = lightBins;
        pipeline.ps.ssao = args.ssao;
        
        if (args.lights->count() > 0)
        {
            pipeline.ps.lightWorldPosition = args.lights->worldPositions();
            pipeline.ps.lightDirection = args.lights->directions();
            pipeline.ps.lightColor = args.lights->colors();
            pipeline.ps.lightIntensity = args.lights->intensities();
            pipeline.ps.lightRange = args.lights->ranges();
            pipeline.ps.lightType = args.lights->types();
            pipeline.ps.lightParameters = args.lights->parameters();
        }
        


        args.cmd->bindPipe(pipeline);
        args.cmd->executeIndexedIndirect(
            indexes.indexBuffer,
            draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
            draws.clusterRendererExecuteIndirectCount.buffer, 0);
    }
}
