#include "engine/rendering/ModelRenderer.h"
#include "engine/rendering/ShadowRenderer.h"
#include "engine/rendering/DepthPyramid.h"
#include "engine/rendering/ImguiRenderer.h"
#include "engine/rendering/BufferSettings.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/SwapChain.h"
#include "shaders/core/shared_types/DebugModes.hlsli"
#include "tools/Measure.h"
#include "tools/ToolsCommon.h"
#include "components/TerrainComponent.h"

#include <random>

using namespace tools;

namespace engine
{
    bool frustumCull(
        engine::vector<Vector4f>& frustumPlanes,
        const BoundingBox& aabb,
        const Vector3f cameraPosition,
        const Matrix4f& transform)
    {
        Vector3f corner[8] =
        {
            transform * aabb.min,
            transform * Vector3f{ aabb.min.x, aabb.max.y, aabb.max.z },
            transform * Vector3f{ aabb.min.x, aabb.min.y, aabb.max.z },
            transform * Vector3f{ aabb.min.x, aabb.max.y, aabb.min.z },
            transform * aabb.max,
            transform * Vector3f{ aabb.max.x, aabb.min.y, aabb.min.z },
            transform * Vector3f{ aabb.max.x, aabb.max.y, aabb.min.z },
            transform * Vector3f{ aabb.max.x, aabb.min.y, aabb.max.z }
        };

        for (int i = 0; i < 6; ++i)
        {
            bool hit = false;
            for (auto&& c : corner)
            {
                if (frustumPlanes[i].xyz().normalize().dot(c - cameraPosition) >= 0)
                {
                    hit = true;
                    break;
                }
            }
            if (!hit)
                return false;
        }
        return true;
    }

    ModelRenderer::ModelRenderer(Device& device, Vector2<int> virtualResolution)
        : m_device{ device }
        , m_bufferMath{ device }
        , m_transparent{ device }
		, m_shapeRenderer{ device }
		, m_renderDepth{ device }
		, m_renderForward{ device }
		, m_renderGbuffer{ device }
		, m_renderDxr{ device }
		, m_ssrRenderer{ device }
#ifdef ECS_TEST_ENABLED
		, m_ecsDemo{ device }
#endif
        , m_clusterDataLine{ { device }, { device }, { device }, { device }, { device }, { device }, { device }, { device } }
        , m_currentDataLine{ 0u }
        , m_indexDataLine{ device }
        , m_drawDataLine{ { device }, { device }, { device }, { device }, { device }, { device }, { device }, { device } }
        , m_phase1{ device }
        , m_phase2{ device }
        
        , m_virtualResolution{ virtualResolution }
        , m_frustumCuller{ device }
        , m_occlusionCuller{ device }
        , m_indexExpansion{ device }

        , m_sceneVoxelizer{ nullptr }
        
        , m_picker{ m_device }
        , m_statistics{}
        , m_modelInstanceCount{ 0u }
        , m_activeClusterCount{ 0u }
        , m_activeIndexCount{ 0u }
#ifdef PARTICLE_TEST_ENABLED
        , m_particleTest{ device }
#endif

        , m_lightingPipeline{ device.createPipeline<shaders::Lighting>() }
		, m_debugLightingPipeline{ device.createPipeline<shaders::Lighting>() }
        , m_lightingPipelineVoxelReflections{ device.createPipeline<shaders::Lighting>() }
        , m_debugLightingPipelineVoxelReflections{ device.createPipeline<shaders::Lighting>() }
        , m_ssaoPipeline{ device.createPipeline<shaders::SSAO>() }
		, m_ssaoForwardPipeline{ device.createPipeline<shaders::SSAOForward>() }
        , m_ssaoBlurPipeline{ device.createPipeline<shaders::SSAOBlur>() }
        , m_temporalResolve{ }
        , m_currentRenderMode{ 0 }
        , m_renderWidth{ m_virtualResolution.x }
        , m_renderHeight{ m_virtualResolution.y }

        , m_renderOutline{ engine::make_unique<RenderOutline>(device, m_virtualResolution) }

        , m_gbuffer{ engine::make_shared<GBuffer>(device, m_renderWidth, m_renderHeight) }

        , m_lightingTarget{ device.createTextureRTV(TextureDescription()
            .width(m_renderWidth)
            .height(m_renderHeight)
            .format(Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("Lighting target")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f })) }
        , m_lightingTargetUAV{ device.createTextureUAV(m_lightingTarget) }
        , m_lightingTargetSRV{ device.createTextureSRV(m_lightingTarget) }

        , m_fullResTargetFrame{ 
            device.createTextureRTV(TextureDescription()
                .width(m_renderWidth)
                .height(m_renderHeight)
                .format(Format::R16G16B16A16_FLOAT)
                .usage(ResourceUsage::GpuRenderTargetReadWrite)
                .name("FullRes Target 0")
                .dimension(ResourceDimension::Texture2D)
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f })),
            device.createTextureRTV(TextureDescription()
                .width(m_renderWidth)
                .height(m_renderHeight)
                .format(Format::R16G16B16A16_FLOAT)
                .usage(ResourceUsage::GpuRenderTargetReadWrite)
                .name("FullRes Target 1")
                .dimension(ResourceDimension::Texture2D)
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }))
        }
		, m_fullResTargetFrameUAV{
            device.createTextureUAV(m_fullResTargetFrame[0]),
            device.createTextureUAV(m_fullResTargetFrame[1])
        }
        , m_fullResTargetFrameSRV{
            device.createTextureSRV(m_fullResTargetFrame[0]),
            device.createTextureSRV(m_fullResTargetFrame[1])
        }
        , m_fullResTransparencyTargetFrame{
            device.createTextureRTV(TextureDescription()
                .width(m_renderWidth)
                .height(m_renderHeight)
                .format(Format::R16G16B16A16_FLOAT)
                .usage(ResourceUsage::GpuRenderTargetReadWrite)
                .name("FullRes Transparency Target 0")
                .dimension(ResourceDimension::Texture2D)
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f })),
            device.createTextureRTV(TextureDescription()
                .width(m_renderWidth)
                .height(m_renderHeight)
                .format(Format::R16G16B16A16_FLOAT)
                .usage(ResourceUsage::GpuRenderTargetReadWrite)
                .name("FullRes Transparency Target 1")
                .dimension(ResourceDimension::Texture2D)
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }))
        
		}
		, m_fullResTransparencyTargetFrameUAV{
            device.createTextureUAV(m_fullResTransparencyTargetFrame[0]),
            device.createTextureUAV(m_fullResTransparencyTargetFrame[1])
        }
        , m_fullResTransparencyTargetFrameSRV{
            device.createTextureSRV(m_fullResTransparencyTargetFrame[0]),
            device.createTextureSRV(m_fullResTransparencyTargetFrame[1])
        }
		
        , m_currentFullResIndex{ 0 }

        , m_ssaoRTV{ device.createTextureRTV(TextureDescription()
#ifdef SCALEAOSIZE
            .width(virtualResolution.x / SSAOresolutionScaleFactor)
            .height(virtualResolution.y / SSAOresolutionScaleFactor)
#else
            .width(SSAOWidth)
            .height(SSAOHeight)
#endif
            .format(Format::R16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("SSAO Render target")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 0.0f })
        ) }
        , m_lastResolvedIndex{ 0 }
        , m_ssaoSRV{ device.createTextureSRV(m_ssaoRTV) }

        /*, m_pickBufferUAV{ device.createBufferUAV(BufferDescription()
            .elements(1)
            .format(Format::R32_UINT)
            .usage(ResourceUsage::GpuReadWrite)
            .name("Pick buffer")
        ) }
        , m_pickBufferReadBack{ device.createBufferSRV(BufferDescription()
            .elements(1)
            .elementSize(sizeof(unsigned int))
            .format(Format::R32_UINT)
            .usage(ResourceUsage::GpuToCpu)
            .name("Pick buffer readback")
        ) }*/

        , m_dsv{ device.createTextureDSV(TextureDescription()
            .name("Depth Buffer")
            .format(Format::D32_FLOAT)
            .width(virtualResolution.x)
            .height(virtualResolution.y)
            .usage(ResourceUsage::DepthStencil)
            .optimizedDepthClearValue(0.0f)
            .dimension(ResourceDimension::Texture2D)
        ) }
        , m_dsvSRV{ device.createTextureSRV(m_dsv) }

        , m_selectedObject{ -1 }
        , m_taaEnabled{ true }
        , m_ssaoEnabled{ true }
        , m_ssrEnabled{ false }
        , m_vsyncEnabled{ true }
        , m_gpuMeasuresEnabled{ false }
        , m_gpuBufferStatsEnabled{ false }
        , m_gpuCullingStatsEnabled{ false }
        , m_logEnabled{ false }
        , m_debugMenuOpen{ true }
		, m_forwardRendering{ true }
        , m_voxelize{ false }
        , m_debugVoxels{ false }
        , m_debugVoxelGrids{ false }
        , m_debugVoxelMip{ 0 }
        , m_renderTransparency{ false }
        , m_histogramDebug{ false }
		, m_debugBoundingBoxes{ false }
		, m_terrainAllDebugBoundingBoxes{ false }
		, m_virtualResolutionImgui{ m_virtualResolution.x, m_virtualResolution.y }
		, m_virtualResolutionImguiChanged{ false }
        , m_terrainDebugMode{ 0 }

        , m_frameStatistics{
            device.createBuffer(BufferDescription()
            .usage(ResourceUsage::GpuToCpu)
                .name("Statistics Buffer")
                .elements(sizeof(FrameStatistics) / sizeof(uint32_t))
                .elementSize(formatBytes(Format::R32_UINT))) }

		//, m_testShape{ ShapeMeshFactory::createCube(Vector3f{0.0f, 0.0f, 0.0f}, Vector3f{1.0f, 1.0f, 1.0f}) }
		//, m_testShape{ ShapeMeshFactory::createSphere(Vector3f{0.0f, 0.0f, 0.0f}, 2.0f, 30, 30) }
		, m_testShape{ ShapeMeshFactory::createSpot(Vector3f{0.0f, 0.0f, 0.0f}, {}, 3, 30, 40) }
		, m_testGpuShape{ device, m_testShape }
		, m_ssrDebug{ device.createPipeline<shaders::SSRDebug>() }
        , m_debugDepth{}
        , m_debugDepthSRV{}
    {
        createSSAOSampleKernel();
        createSSAOBlurKernel();
        m_ssaoNoiseTexture = device.createTextureSRV(TextureDescription()
            .width(4)
            .height(4)
            .format(Format::R32G32B32A32_FLOAT)
            .name("SSAO noise texture")
            .setInitialData(TextureDescription::InitialData(
                m_ssaoNoise, 
                4u * static_cast<uint32_t>(4 * sizeof(float)), 
                16u * static_cast<uint32_t>(4 * sizeof(float))))
        );
        m_ssaoKernelBuffer = device.createBufferSRV(BufferDescription()
            .elementSize(sizeof(Float4))
            .elements(m_ssaoKernel.size())
            .name("SSAO Kernel buffer")
            .setInitialData(m_ssaoKernel)
            .format(Format::R32G32B32A32_FLOAT)
        );
        m_ssaoBlurKernelBuffer = device.createBufferSRV(BufferDescription()
            .elementSize(sizeof(Float))
            .elements(m_ssaoBlurKernel.size())
            .name("SSAO Blur kernel buffer")
            .setInitialData(m_ssaoBlurKernel)
            .format(Format::R32_FLOAT)
        );

        m_ssaoPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_ssaoPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_ssaoPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_ssaoPipeline.ps.noiseTexture = m_ssaoNoiseTexture;
        m_ssaoPipeline.ps.ssaoSampler = device.createSampler(SamplerDescription()
            .textureAddressMode(TextureAddressMode::Wrap)
            .filter(Filter::Point)
        );
        m_ssaoPipeline.ps.depthSampler = device.createSampler(SamplerDescription().textureAddressMode(TextureAddressMode::Clamp).filter(Filter::Point));
        m_ssaoPipeline.ps.defaultSampler = device.createSampler(SamplerDescription().filter(Filter::Point));

		m_ssaoForwardPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
		m_ssaoForwardPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
		m_ssaoForwardPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
		m_ssaoForwardPipeline.ps.noiseTexture = m_ssaoNoiseTexture;
		m_ssaoForwardPipeline.ps.ssaoSampler = device.createSampler(SamplerDescription()
			.textureAddressMode(TextureAddressMode::Wrap)
			.filter(Filter::Point));
		m_ssaoForwardPipeline.ps.depthSampler = device.createSampler(SamplerDescription().textureAddressMode(TextureAddressMode::Clamp).filter(Filter::Point));
        m_ssaoForwardPipeline.ps.defaultSampler = device.createSampler(SamplerDescription().textureAddressMode(TextureAddressMode::Clamp).filter(Filter::Point));

        m_ssaoBlurPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_ssaoBlurPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_ssaoBlurPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_ssaoBlurPipeline.ps.imageSampler = device.createSampler(SamplerDescription().filter(engine::Filter::Point).textureAddressMode(TextureAddressMode::Clamp));

		m_ssrDebug.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
		m_ssrDebug.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
		m_ssrDebug.setDepthStencilState(DepthStencilDescription().depthEnable(false));

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
        m_lightingPipeline = device.createPipeline<shaders::Lighting>();
        m_lightingPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_lightingPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_lightingPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_lightingPipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
        m_lightingPipeline.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
        m_lightingPipeline.ps.depth_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
        m_lightingPipeline.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
        m_lightingPipeline.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
        m_lightingPipeline.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
        m_lightingPipeline.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
        m_lightingPipeline.ps.frameSize.x = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).width());
        m_lightingPipeline.ps.frameSize.y = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).height());
        m_lightingPipeline.ps.materialTextures = device.modelResources().textures();
		m_lightingPipeline.ps.debug = false;
        m_lightingPipeline.ps.voxelreflections = false;
        m_lightingPipeline.ps.shadow_sampler = device.createSampler(SamplerDescription()
            .addressU(TextureAddressMode::Mirror)
            .addressV(TextureAddressMode::Mirror)
            .filter(Filter::Comparison));

		m_debugLightingPipeline = device.createPipeline<shaders::Lighting>();
		m_debugLightingPipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
		m_debugLightingPipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
		m_debugLightingPipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
		m_debugLightingPipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
		m_debugLightingPipeline.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
		m_debugLightingPipeline.ps.depth_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
		m_debugLightingPipeline.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
		m_debugLightingPipeline.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
		m_debugLightingPipeline.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
		m_debugLightingPipeline.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
		m_debugLightingPipeline.ps.frameSize.x = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).width());
		m_debugLightingPipeline.ps.frameSize.y = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).height());
		m_debugLightingPipeline.ps.materialTextures = device.modelResources().textures();
		m_debugLightingPipeline.ps.debug = true;
        m_debugLightingPipeline.ps.voxelreflections = false;
		m_debugLightingPipeline.ps.shadow_sampler = device.createSampler(SamplerDescription()
			.addressU(TextureAddressMode::Mirror)
			.addressV(TextureAddressMode::Mirror)
			.filter(Filter::Comparison));

        // lighting
        m_lightingPipelineVoxelReflections = device.createPipeline<shaders::Lighting>();
        m_lightingPipelineVoxelReflections.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_lightingPipelineVoxelReflections.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_lightingPipelineVoxelReflections.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_lightingPipelineVoxelReflections.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
        m_lightingPipelineVoxelReflections.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
        m_lightingPipelineVoxelReflections.ps.depth_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
        m_lightingPipelineVoxelReflections.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
        m_lightingPipelineVoxelReflections.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
        m_lightingPipelineVoxelReflections.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
        m_lightingPipelineVoxelReflections.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
        m_lightingPipelineVoxelReflections.ps.frameSize.x = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).width());
        m_lightingPipelineVoxelReflections.ps.frameSize.y = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).height());
        m_lightingPipelineVoxelReflections.ps.materialTextures = device.modelResources().textures();
        m_lightingPipelineVoxelReflections.ps.debug = false;
        m_lightingPipelineVoxelReflections.ps.voxelreflections = true;
        m_lightingPipelineVoxelReflections.ps.shadow_sampler = device.createSampler(SamplerDescription()
            .addressU(TextureAddressMode::Mirror)
            .addressV(TextureAddressMode::Mirror)
            .filter(Filter::Comparison));

        m_debugLightingPipelineVoxelReflections = device.createPipeline<shaders::Lighting>();
        m_debugLightingPipelineVoxelReflections.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_debugLightingPipelineVoxelReflections.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_debugLightingPipelineVoxelReflections.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_debugLightingPipelineVoxelReflections.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
        m_debugLightingPipelineVoxelReflections.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
        m_debugLightingPipelineVoxelReflections.ps.depth_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
        m_debugLightingPipelineVoxelReflections.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
        m_debugLightingPipelineVoxelReflections.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
        m_debugLightingPipelineVoxelReflections.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
        m_debugLightingPipelineVoxelReflections.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
        m_debugLightingPipelineVoxelReflections.ps.frameSize.x = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).width());
        m_debugLightingPipelineVoxelReflections.ps.frameSize.y = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).height());
        m_debugLightingPipelineVoxelReflections.ps.materialTextures = device.modelResources().textures();
        m_debugLightingPipelineVoxelReflections.ps.debug = true;
        m_debugLightingPipelineVoxelReflections.ps.voxelreflections = true;
        m_debugLightingPipelineVoxelReflections.ps.shadow_sampler = device.createSampler(SamplerDescription()
            .addressU(TextureAddressMode::Mirror)
            .addressV(TextureAddressMode::Mirror)
            .filter(Filter::Comparison));

        for (int i = 0; i < 2; ++i)
        {
            auto temporalPipe = device.createPipeline<shaders::TemporalResolve>();
            temporalPipe.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
            temporalPipe.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
            temporalPipe.setDepthStencilState(DepthStencilDescription().depthEnable(false));
            temporalPipe.ps.pointSampler = device.createSampler(SamplerDescription().textureAddressMode(TextureAddressMode::Clamp).filter(Filter::Point));
            temporalPipe.ps.bilinearSampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear));
            temporalPipe.ps.visualizeMotion = static_cast<bool>(i);
            m_temporalResolve.emplace_back(std::move(temporalPipe));
        }

    }

    void ModelRenderer::resize(uint32_t width, uint32_t height)
    {
        m_virtualResolution = { static_cast<int>(width), static_cast<int>(height) };
        m_renderWidth = m_virtualResolution.x;
        m_renderHeight = m_virtualResolution.y;

        m_renderOutline = engine::make_unique<RenderOutline>(m_device, m_virtualResolution);

        m_gbuffer = nullptr;
        m_gbuffer = engine::make_shared<GBuffer>(m_device, m_renderWidth, m_renderHeight);

        m_dsv = m_device.createTextureDSV(TextureDescription()
            .name("Depth Buffer")
            .format(Format::D32_FLOAT)
            .width(m_renderWidth)
            .height(m_renderHeight)
            .usage(ResourceUsage::DepthStencil)
            .optimizedDepthClearValue(0.0f)
            .dimension(ResourceDimension::Texture2D)
        );
        m_dsvSRV = m_device.createTextureSRV(m_dsv);

        m_lightingTarget = m_device.createTextureRTV(TextureDescription()
            .width(m_renderWidth)
            .height(m_renderHeight)
            .format(Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("Lighting target")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));

        m_lightingTargetUAV = m_device.createTextureUAV(m_lightingTarget);
        m_lightingTargetSRV = m_device.createTextureSRV(m_lightingTarget);

        m_fullResTargetFrame[0] = m_device.createTextureRTV(TextureDescription()
            .width(m_renderWidth)
            .height(m_renderHeight)
            .format(Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("FullRes Target 0")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));

        m_fullResTargetFrame[1] = m_device.createTextureRTV(TextureDescription()
            .width(m_renderWidth)
            .height(m_renderHeight)
            .format(Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("FullRes Target 1")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));
        
		m_fullResTargetFrameUAV[0] = m_device.createTextureUAV(m_fullResTargetFrame[0]);
		m_fullResTargetFrameUAV[1] = m_device.createTextureUAV(m_fullResTargetFrame[1]);

        m_fullResTargetFrameSRV[0] = m_device.createTextureSRV(m_fullResTargetFrame[0]);
        m_fullResTargetFrameSRV[1] = m_device.createTextureSRV(m_fullResTargetFrame[1]);
        

        m_fullResTransparencyTargetFrame[0] = m_device.createTextureRTV(TextureDescription()
            .width(m_renderWidth)
            .height(m_renderHeight)
            .format(Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("FullRes Transparency Target 0")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));

        m_fullResTransparencyTargetFrame[1] = m_device.createTextureRTV(TextureDescription()
            .width(m_renderWidth)
            .height(m_renderHeight)
            .format(Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("FullRes Transparency Target 1")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));

		m_fullResTransparencyTargetFrameUAV[0] = m_device.createTextureUAV(m_fullResTransparencyTargetFrame[0]);
		m_fullResTransparencyTargetFrameUAV[1] = m_device.createTextureUAV(m_fullResTransparencyTargetFrame[1]);

        m_fullResTransparencyTargetFrameSRV[0] = m_device.createTextureSRV(m_fullResTransparencyTargetFrame[0]);
        m_fullResTransparencyTargetFrameSRV[1] = m_device.createTextureSRV(m_fullResTransparencyTargetFrame[1]);

        m_currentFullResIndex = 0;
        m_lastResolvedIndex = 0;

        m_ssaoRTV = m_device.createTextureRTV(TextureDescription()
#ifdef SCALEAOSIZE
            .width(m_virtualResolution.x / SSAOresolutionScaleFactor)
            .height(m_virtualResolution.y / SSAOresolutionScaleFactor)
#else
            .width(SSAOWidth)
            .height(SSAOHeight)
#endif
            .format(Format::R16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("SSAO Render target")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 0.0f })
        );
        m_ssaoSRV = m_device.createTextureSRV(m_ssaoRTV);

        m_lightingPipeline.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
        m_lightingPipeline.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
        m_lightingPipeline.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
        m_lightingPipeline.ps.materialTextures = m_device.modelResources().textures();

		m_debugLightingPipeline.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
		m_debugLightingPipeline.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
		m_debugLightingPipeline.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
		m_debugLightingPipeline.ps.materialTextures = m_device.modelResources().textures();

        m_lightingPipelineVoxelReflections.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
        m_lightingPipelineVoxelReflections.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
        m_lightingPipelineVoxelReflections.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
        m_lightingPipelineVoxelReflections.ps.materialTextures = m_device.modelResources().textures();

        m_debugLightingPipelineVoxelReflections.ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
        m_debugLightingPipelineVoxelReflections.ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
        m_debugLightingPipelineVoxelReflections.ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);
        m_debugLightingPipelineVoxelReflections.ps.materialTextures = m_device.modelResources().textures();

        m_debugDepth = {};
        m_debugDepthSRV = {};
    }

    void ModelRenderer::renderDebugMenu()
	{
		// debug options
		{
			float logWidth = 150.0f;
			float logHeight = 100.0f;

			ImGui::SetNextWindowSize(ImVec2(logWidth, logHeight), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
			if (ImGui::Begin("DebugMenu", &m_debugMenuOpen, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Checkbox("GPU Times", &m_gpuMeasuresEnabled);
				ImGui::Checkbox("GPU Buffer Statistics", &m_gpuBufferStatsEnabled);
				ImGui::Checkbox("GPU Culling Statistics", &m_gpuCullingStatsEnabled);
				ImGui::Checkbox("Log enabled", &m_logEnabled);
                ImGui::Checkbox("TAA Enabled", &m_taaEnabled);
				ImGui::Checkbox("SSAO Enabled", &m_ssaoEnabled);
                ImGui::Checkbox("SSR Enabled", &m_ssrEnabled);
				ImGui::Checkbox("Vsync Enabled", &m_vsyncEnabled);
				ImGui::Checkbox("Forward rendering", &m_forwardRendering);
                ImGui::Checkbox("Render transparency", &m_renderTransparency);
                ImGui::Checkbox("Render histogram debug", &m_histogramDebug);
				ImGui::Checkbox("Debug bounding boxes", &m_debugBoundingBoxes);
                if (ImGui::TreeNode("Terrain"))
                {
					ImGui::Checkbox("Show sector bounding boxes", &m_terrainAllDebugBoundingBoxes);
                    ImGui::Combo("Terrain debug mode", &m_terrainDebugMode, "Draw terrain\0Draw wireframe\0Draw terrain and wireframe");
                    ImGui::TreePop();
                }

                ImGui::Checkbox("Voxelize", &m_voxelize);
                if (m_voxelize)
                {
                    ImGui::Checkbox("Debug voxels", &m_debugVoxels);

                    if (m_debugVoxels)
                    {
                        ImGui::Checkbox("Show voxel grids", &m_debugVoxelGrids);
                        ImGui::Combo("Voxel mip", &m_debugVoxelMip, "Mip 0\0Mip 1\0Mip 2\0Mip 3\0Mip 4\0Mip 5\0Mip 6\0Mip 7\0Mip 8");
                    }
                }
				
				if (ImGui::TreeNode("Virtual resolution"))
				{
					//ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
					ImGui::BeginChild(565, { 540, 85 });
					ImGui::Text("Virtual resolution");
					ImGui::InputInt("Width", &m_virtualResolutionImgui[0]); ImGui::InputInt("Height", &m_virtualResolutionImgui[1]); ImGui::SameLine();
					if (ImGui::Button("Set", { 60, 35 }))
					{
						m_virtualResolution = { m_virtualResolutionImgui[0], m_virtualResolutionImgui[1] };
						m_virtualResolutionImguiChanged = true;
					}
					ImGui::EndChild();
                    ImGui::TreePop();
				}
				if (ImGui::TreeNode("Rendering"))
				{
					ImGui::Combo("Mode", &m_currentRenderMode, "Full\0Clusters\0MipAlbedo\0MipRoughness\0MipMetalness\0MipAO\0Albedo\0Roughness\0Metalness\0Occlusion\0Uv\0Normal\0Motion\0LightBins\0Triangles\0");
					ImGui::TreePop();
				}

			}
			ImGui::End();

			auto swapChain = m_device.currentSwapChain().lock();
			if (swapChain)
			{
				swapChain->vsync(m_vsyncEnabled);
			}
		}
	}

	Vector2<int> ModelRenderer::virtualResolution() const
	{
		return { m_virtualResolutionImgui[0], m_virtualResolutionImgui[1] };
	}

	bool ModelRenderer::virtualResolutionChange(bool reset)
	{
		bool res = m_virtualResolutionImguiChanged;

		if (reset)
		{
			m_virtualResolutionImguiChanged = false;
		}
		return res;
	}

	ModelRenderer::DataLineSetup ModelRenderer::setupDataLines()
	{
		DataLineSetup dataline;
		dataline.currentDataLine = m_currentDataLine;
		dataline.previousDataLine = (m_currentDataLine - 1) % DataLineCount;
		dataline.alphaClipDataLine = 2;
		dataline.alphaClipDataLineSecond = 3;
		dataline.transparentDataLine = 4;
		dataline.transparentDataLineSecond = 5;
		dataline.terrainDataLine = 6;
		dataline.terrainDataLineSecond = 7;
		return dataline;
	}

    void ModelRenderer::render(
        Device& device,
        DepthPyramid& depthPyramid,
        CommandList& cmd,
        FlatScene& scene,
        // these are for voxel lighting
        TextureSRV shadowMap,
        BufferSRV shadowVP,
        BufferSRV lightIndexToShadowIndex,
        LightData& lights)
    {
        CPU_MARKER(cmd.api(), "Render culled scene");
        GPU_MARKER(cmd, "Render culled scene");

        reallocateClusterDatalines();

		renderDebugMenu();
		
        if(!scene.validScene())
            return;

        auto frameNumber = device.frameNumber();
        
        m_viewMatrix = scene.drawCamera().viewMatrix();
        m_projectionMatrix = scene.drawCamera().projectionMatrix(m_virtualResolution);
        m_jitterMatrix = scene.drawCamera().jitterMatrix(frameNumber, m_virtualResolution);
        m_jitterValue = scene.drawCamera().jitterValue(frameNumber);

		DataLineSetup dataline = setupDataLines();

		for (auto&& terrain : scene.terrains)
		{
			terrain->terrain().updateTerrainPatches(cmd, scene.drawCamera(), m_terrainAllDebugBoundingBoxes);
		}

        // -----------------------------------------------------------------------------------------------------------------------------
        // ------------------------------     PHASE 1     ------------------------------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
		resetDataLines(cmd, dataline);
		clearDepth(cmd, depthPyramid);
		frustumCullPreviousClusters(cmd, dataline, scene.cullingCamera());

        if(scene.cameraDebugging())
		    drawFirstPassDeferredDebug(cmd, dataline, depthPyramid, scene.drawCamera(), scene.cullingCamera(), scene);
        else
            drawFirstPassDeferred(cmd, dataline, depthPyramid, scene.drawCamera(), scene, shadowMap, shadowVP, lightIndexToShadowIndex, lights);
		createDepthPyramid(cmd, depthPyramid);
		copyFirstPassStatistics(cmd, dataline);
        
        // -----------------------------------------------------------------------------------------------------------------------------
        // ------------------------------     PHASE 2     ------------------------------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
		frustumInstanceCull(cmd, dataline, depthPyramid, scene.cullingCamera());
		expandClusters(cmd);
		occlusionCull(cmd, dataline, depthPyramid, scene.cullingCamera());
        if (scene.cameraDebugging())
		    drawSecondPassDeferredDebug(cmd, dataline, scene.drawCamera());
        else
            drawSecondPassDeferred(cmd, dataline, depthPyramid, scene.drawCamera());
		copySecondPassStatistics(cmd, dataline);
		resetBloomFilter(cmd);
    }

	void ModelRenderer::renderForward(
		Device& device,
		DepthPyramid& depthPyramid,
		CommandList& cmd,
		FlatScene& scene,
		TextureSRV shadowMap,
		BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
		LightData& lights)
	{
		CPU_MARKER(cmd.api(), "Forward Render culled scene");
		GPU_MARKER(cmd, "Forward Render culled scene");
#if 1
		Vector3f probePosition;
		float probeRange = 100.0f;
		if (scene.probes.size() > 0)
		{
			ProbeComponent& probe = *scene.probes[0];
			probePosition = probe.position();
			probeRange = probe.range();
		}

        reallocateClusterDatalines();

		renderDebugMenu();

        if (!scene.validScene())
            return;

		auto frameNumber = device.frameNumber();

        auto& camera = scene.drawCamera();

		m_viewMatrix = camera.viewMatrix();
		m_projectionMatrix = camera.projectionMatrix(m_virtualResolution);
		m_jitterMatrix = camera.jitterMatrix(frameNumber, m_virtualResolution);
		m_jitterValue = camera.jitterValue(frameNumber);
#endif
		DataLineSetup dataline = setupDataLines();

		for (auto&& terrain : scene.terrains)
		{
			terrain->terrain().updateTerrainPatches(cmd, scene.drawCamera(), m_terrainAllDebugBoundingBoxes);
		}

		// -----------------------------------------------------------------------------------------------------------------------------
		// ------------------------------     PHASE 1     ------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------------------------------------------------
		resetDataLines(cmd, dataline);
		clearDepth(cmd, depthPyramid);
		frustumCullPreviousClusters(cmd, dataline, scene.cullingCamera());
        
        if (scene.cameraDebugging())
		    drawFirstPassForwardDebug(cmd, dataline, depthPyramid, scene, shadowMap, shadowVP, lightIndexToShadowIndex, lights, probePosition, probeRange);
        else
            drawFirstPassForward(cmd, dataline, depthPyramid, scene, shadowMap, shadowVP, lightIndexToShadowIndex, lights, probePosition, probeRange);
		//createDepthPyramid(cmd, depthPyramid);
		copyFirstPassStatistics(cmd, dataline);

		// -----------------------------------------------------------------------------------------------------------------------------
		// ------------------------------     PHASE 2     ------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------------------------------------------------
		frustumInstanceCull(cmd, dataline, depthPyramid, scene.cullingCamera());
		expandClusters(cmd);
		occlusionCull(cmd, dataline, depthPyramid, scene.cullingCamera());
        if (scene.cameraDebugging())
		    drawSecondPassForwardDebug(cmd, dataline, depthPyramid, camera, shadowMap, shadowVP, lightIndexToShadowIndex, lights, probePosition, probeRange, scene);
        else
            drawSecondPassForward(cmd, dataline, depthPyramid, camera, shadowMap, shadowVP, lightIndexToShadowIndex, lights, probePosition, probeRange, scene);
		copySecondPassStatistics(cmd, dataline);
		resetBloomFilter(cmd);
	}

    void ModelRenderer::reallocateClusterDatalines()
    {
        auto newClusterCount = m_device.modelResources().clusterCount() * 10;

        if (m_activeClusterCount != newClusterCount)
        {
            for (int i = 0; i < 8; ++i)
                m_clusterDataLine[i].resize(m_device, newClusterCount);

            m_activeClusterCount = newClusterCount;
        }

        if (m_activeIndexCount < m_device.modelResources().indexCount())
        {
            m_activeIndexCount = m_device.modelResources().indexCount();
            m_indexDataLine.resize(m_device, m_activeIndexCount);
        }
    }

	void ModelRenderer::renderFrameDownsample(
		CommandList& cmd)
	{
		m_transparent.createDownsamples(cmd, getCurrentLightingTargetSRV());
	}

	TextureSRV ModelRenderer::ssrResult()
	{
		return m_ssrRenderer.ssr();
	}

	void ModelRenderer::renderSSR(
		Device& device,
		CommandList& cmd,
		DepthPyramid& depthPyramid,
        FlatScene& scene)
	{
        if(m_ssrEnabled)
		    m_ssrRenderer.render(device, cmd, depthPyramid, getCurrentLightingTargetSRV() /*m_fullResTargetFrameSRV[m_lastResolvedIndex]*/, *m_gbuffer, scene.drawCamera());
	}

	void ModelRenderer::renderSSRForward(
		Device& device,
		CommandList& cmd,
		DepthPyramid& depthPyramid,
		Camera& camera)
	{
        if(m_ssrEnabled)
		    m_ssrRenderer.renderForward(device, cmd, depthPyramid.srv(), getCurrentLightingTargetSRV() /*m_fullResTargetFrameSRV[m_lastResolvedIndex]*/, camera);
	}

    void ModelRenderer::renderTransparent(
        Device& device,
        DepthPyramid& depthPyramid,
        CommandList& cmd,
        FlatScene& scene,
        TextureSRV shadowMap,
        BufferSRV shadowVP)
    {
        if (!m_renderTransparency)
            return;

        CPU_MARKER(cmd.api(), "Render transparency");
        GPU_MARKER(cmd, "Render transparency");

        auto& camera = scene.drawCamera();

        // lighting
        Vector3f probePosition;
        float probeRange = 100.0f;
        if (scene.probes.size() > 0)
        {
            ProbeComponent& probe = *scene.probes[0];
            probePosition = probe.position();
            probeRange = probe.range();
        }

        ClusterRendererTransparencyArgs transparentArgs;
        transparentArgs.cmd = &cmd;
        transparentArgs.camera = &camera;

        transparentArgs.target = m_fullResTransparencyTargetFrame[0];
        if(scene.cameraDebugging() && m_debugDepth)
            transparentArgs.depthBuffer = m_debugDepth;
        else
            transparentArgs.depthBuffer = depthPyramid.dsv();
        transparentArgs.depthView = depthPyramid.srv();
        transparentArgs.ssao = m_blurTargetSRV;
        transparentArgs.motion = m_gbuffer->srv(GBufferType::Motion);

        transparentArgs.frameNumber = device.frameNumber();
        transparentArgs.virtualResolution = m_virtualResolution;

        transparentArgs.previousViewMatrix = m_previousCameraViewMatrix;
        transparentArgs.previousProjectionMatrix = m_previousCameraProjectionMatrix;

        transparentArgs.shadowMap = &shadowMap;
        transparentArgs.shadowVP = &shadowVP;

        transparentArgs.cameraProjectionMatrix = camera.projectionMatrix();
        transparentArgs.cameraViewMatrix = camera.viewMatrix();
        transparentArgs.lights = scene.lightData.get();
        transparentArgs.probePosition = probePosition;
        transparentArgs.probeRange = probeRange;

        transparentArgs.currentRenderMode = m_currentRenderMode;

		// initialize transparency result with current frame data
		{
            CPU_MARKER(cmd.api(), "Copy frame to transparency");
            GPU_MARKER(cmd, "Copy frame to transparency");
			cmd.copyTexture(getCurrentLightingTargetSRV(), m_fullResTransparencyTargetFrameUAV[0]);
		}

        // 10. Draw transparency 1st pass
		{
			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDrawTransparent,
				m_indexDataLine,
				m_phase1.transparentIndexDraw,
				m_drawDataLine[4]);

			m_transparent.render(
				transparentArgs,
				m_phase1.frustumClusterCullDrawTransparent,
				m_phase1.transparentIndexDraw,
				m_drawDataLine[4],
				m_shapeRenderer.lightBins(),
                getCurrentLightingTargetSRV());
		}

		// initialize transparency result with current frame data
		{
            CPU_MARKER(cmd.api(), "Copy frame to transparency");
            GPU_MARKER(cmd, "Copy frame to transparency");
			cmd.copyTexture(m_fullResTransparencyTargetFrameSRV[0], m_fullResTransparencyTargetFrameUAV[1]);
		}

		// 10. Draw transparency 2nd pass
		{
            m_indexExpansion.expandIndexes(m_device, cmd,
                m_phase2.occlusionCullDrawTransparent,
                m_indexDataLine,
                m_phase2.transparentIndexDraw,
                m_drawDataLine[5]);

            transparentArgs.target = m_fullResTransparencyTargetFrame[1];
            m_transparent.render(
                transparentArgs,
                m_phase2.occlusionCullDrawTransparent,
                m_phase2.transparentIndexDraw,
                m_drawDataLine[5],
				m_shapeRenderer.lightBins(),
                m_fullResTransparencyTargetFrameSRV[0]);
        }

        {
            CPU_MARKER(cmd.api(), "Copy transparency back to frame");
            GPU_MARKER(cmd, "Copy transparency back to frame");
            cmd.copyTexture(m_fullResTransparencyTargetFrameSRV[1], getCurrentLightingTargetUAV());
        }


    }

	void ModelRenderer::resetDataLines(CommandList& cmd, DataLineSetup dataline)
	{
		CPU_MARKER(cmd.api(), "0. Reset datalines");
		GPU_MARKER(cmd, "0. Reset datalines");

		// 0. reset new data line
		m_clusterDataLine[dataline.currentDataLine].reset(cmd);
		m_clusterDataLine[dataline.alphaClipDataLine].reset(cmd);
		m_clusterDataLine[dataline.transparentDataLine].reset(cmd);
		m_clusterDataLine[dataline.terrainDataLine].reset(cmd);
		m_indexDataLine.reset(cmd);
		for (int i = 0; i < 6; ++i)
			m_drawDataLine[i].reset(cmd);

		m_gbuffer->clear(cmd);
		m_frustumCuller.clearInstanceCount(cmd);
	}

	void ModelRenderer::clearDepth(CommandList& cmd, DepthPyramid& depthPyramid)
	{
		CPU_MARKER(cmd.api(), "Clear depth");
		GPU_MARKER(cmd, "Clear depth");
		cmd.clearDepthStencilView(depthPyramid.dsv(), 0.0f);
	}

	void ModelRenderer::frustumCullPreviousClusters(CommandList& cmd, DataLineSetup dataline, Camera& cullingCamera)
	{
		// 1. Cluster frustum cull
		m_frustumCuller.clusterCull(
			cmd, cullingCamera, m_device.modelResources(), m_virtualResolution,
			m_phase2.occlusionCullDrawAll,//m_clusterDataLine[previousDataLine],    // input

			m_clusterDataLine[dataline.currentDataLine],     // append output
			m_clusterDataLine[dataline.alphaClipDataLine],   // append alphaClipped output
			m_clusterDataLine[dataline.transparentDataLine],   // append transparent output
			m_clusterDataLine[dataline.terrainDataLine],   // append terrain output

			m_phase1.frustumClusterCullDraw,
			m_phase1.frustumClusterCullDrawAlphaClipped,
			m_phase1.frustumClusterCullDrawTransparent,
			m_phase1.frustumClusterCullDrawTerrain,

            m_device.modelResources().gpuBuffers().instanceClusterTracking(),
            m_device.modelResources().gpuBuffers().clusterTrackingClusterInstancesUAV);
	}

	void ModelRenderer::drawFirstPassDeferred(
        CommandList& cmd, 
        DataLineSetup dataline, 
        DepthPyramid& depthPyramid, 
        Camera& drawCamera, 
        FlatScene& scene,
        // these are for voxel lighting
        TextureSRV shadowMap,
        BufferSRV shadowVP,
        BufferSRV lightIndexToShadowIndex,
        LightData& lights)
	{
		// 3. Draw depth
		CPU_MARKER(cmd.api(), "Opaque pass");
		GPU_MARKER(cmd, "Opaque pass");

		{
			CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
			GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDraw,
				m_indexDataLine,
				m_phase1.depthIndexDraw,
				m_drawDataLine[0]);

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDrawAlphaClipped,
				m_indexDataLine,
				m_phase1.alphaClipIndexDraw,
				m_drawDataLine[dataline.alphaClipDataLine]);

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDrawTerrain,
				m_indexDataLine,
				m_phase1.terrainIndexDraw,
				m_drawDataLine[dataline.terrainDataLine],
				600u);
		}

		{
			CPU_MARKER(cmd.api(), "Z-Prepass for alphaclipped");
			GPU_MARKER(cmd, "Z-Prepass for alphaclipped");

			m_renderDepth.render(
				cmd,
				m_phase1.frustumClusterCullDrawAlphaClipped,
				m_phase1.alphaClipIndexDraw,
				m_drawDataLine[dataline.alphaClipDataLine],
				depthPyramid.dsv(),
                drawCamera,
				JitterOption::Enabled,
				AlphaClipOption::Enabled,
				BiasOption::Disabled,
				m_virtualResolution);
		}

        renderVoxels(cmd, scene, shadowMap, shadowVP, lightIndexToShadowIndex, lights);

		{
			CPU_MARKER(cmd.api(), "GBuffer pass for opaque and alphaclipped");
			GPU_MARKER(cmd, "GBuffer pass for opaque and alphaclipped");

			m_renderGbuffer.render(
				cmd, 
				*m_gbuffer.get(),
				m_phase1.frustumClusterCullDraw,
				m_phase1.depthIndexDraw,
				m_drawDataLine[0], 
				depthPyramid.dsv(), 
				drawCamera, 
				JitterOption::Enabled, 
				AlphaClipOption::Disabled, 
                m_currentRenderMode,
				BiasOption::Disabled, 
				DepthTestOption::GreaterEqual,
				m_virtualResolution,
				m_previousCameraViewMatrix,
				m_previousCameraProjectionMatrix);

			m_renderGbuffer.render(
				cmd,
				*m_gbuffer.get(),
				m_phase1.frustumClusterCullDrawAlphaClipped,
				m_phase1.alphaClipIndexDraw,
				m_drawDataLine[dataline.alphaClipDataLine],
				depthPyramid.dsv(),
				drawCamera,
				JitterOption::Enabled,
				AlphaClipOption::Enabled,
                m_currentRenderMode,
				BiasOption::Disabled,
				DepthTestOption::Equal,
				m_virtualResolution,
				m_previousCameraViewMatrix,
				m_previousCameraProjectionMatrix);

			m_renderGbuffer.render(
				cmd,
				*m_gbuffer.get(),
				m_phase1.frustumClusterCullDrawTerrain,
				m_phase1.terrainIndexDraw,
				m_drawDataLine[dataline.terrainDataLine],
				depthPyramid.dsv(),
				drawCamera,
				JitterOption::Enabled,
				AlphaClipOption::Disabled,
				m_currentRenderMode,
				BiasOption::Disabled,
				DepthTestOption::GreaterEqual,
				m_virtualResolution,
				m_previousCameraViewMatrix,
				m_previousCameraProjectionMatrix);
		}
	}

    void ModelRenderer::drawFirstPassDeferredDebug(
        CommandList& cmd, 
        DataLineSetup dataline, 
        DepthPyramid& depthPyramid, 
        Camera& drawCamera, 
        Camera& cullCamera,
		FlatScene& scene)
    {
        // 3. Draw depth
        CPU_MARKER(cmd.api(), "Opaque pass");
        GPU_MARKER(cmd, "Opaque pass");

        {
            CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
            GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");

            m_indexExpansion.expandIndexes(m_device, cmd,
                m_phase1.frustumClusterCullDraw,
                m_indexDataLine,
                m_phase1.depthIndexDraw,
                m_drawDataLine[0]);

            m_indexExpansion.expandIndexes(m_device, cmd,
                m_phase1.frustumClusterCullDrawAlphaClipped,
                m_indexDataLine,
                m_phase1.alphaClipIndexDraw,
                m_drawDataLine[dataline.alphaClipDataLine]);

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDrawTerrain,
				m_indexDataLine,
				m_phase1.terrainIndexDraw,
				m_drawDataLine[dataline.terrainDataLine],
				600u);
        }

        {
            CPU_MARKER(cmd.api(), "Write depth for opaque and alphaclipped");
            GPU_MARKER(cmd, "Write depth for opaque and alphaclipped");

            m_renderDepth.render(
                cmd,
                m_phase1.frustumClusterCullDraw,
                m_phase1.depthIndexDraw,
                m_drawDataLine[0],
                depthPyramid.dsv(),
                cullCamera,
                JitterOption::Enabled,
                AlphaClipOption::Disabled,
                BiasOption::Disabled,
                m_virtualResolution);

            m_renderDepth.render(
                cmd,
                m_phase1.frustumClusterCullDrawAlphaClipped,
                m_phase1.alphaClipIndexDraw,
                m_drawDataLine[dataline.alphaClipDataLine],
                depthPyramid.dsv(),
                cullCamera,
                JitterOption::Enabled,
                AlphaClipOption::Enabled,
                BiasOption::Disabled,
                m_virtualResolution);

			if (m_terrainDebugMode != 1)
			{
				for (auto&& terrain : scene.terrains)
				{
					terrain->terrain().renderDepth(
						cmd,
						m_phase1.frustumClusterCullDrawTerrain,
						m_phase1.terrainIndexDraw,
						m_drawDataLine[dataline.terrainDataLine],
						depthPyramid.dsv(),
						cullCamera,
						JitterOption::Enabled,
						BiasOption::Disabled,
						m_virtualResolution);
				}
			}
        }

        if (!m_debugDepth)
        {
            m_debugDepth = m_device.createTextureDSV(TextureDescription()
                .name("Depth Buffer")
                .format(Format::D32_FLOAT)
                .width(depthPyramid.dsv().width())
                .height(depthPyramid.dsv().height())
                .usage(ResourceUsage::DepthStencil)
                .optimizedDepthClearValue(0.0f)
                .dimension(ResourceDimension::Texture2D)
            );
            m_debugDepthSRV = m_device.createTextureSRV(m_debugDepth);
        }

        {
            CPU_MARKER(cmd.api(), "Clear debug depth");
            GPU_MARKER(cmd, "Clear debug depth");
            cmd.clearDepthStencilView(m_debugDepth, 0.0f);
        }

        {
            CPU_MARKER(cmd.api(), "GBuffer pass for debug camera opaque and alphaclipped");
            GPU_MARKER(cmd, "GBuffer pass for debug camera opaque and alphaclipped");

            m_renderGbuffer.render(
                cmd,
                *m_gbuffer.get(),
                m_phase1.frustumClusterCullDraw,
                m_phase1.depthIndexDraw,
                m_drawDataLine[0],
                m_debugDepth,
                drawCamera,
                JitterOption::Enabled,
                AlphaClipOption::Disabled,
                m_currentRenderMode,
                BiasOption::Disabled,
                DepthTestOption::GreaterEqual,
                m_virtualResolution,
                m_previousCameraViewMatrix,
                m_previousCameraProjectionMatrix);

            m_renderGbuffer.render(
                cmd,
                *m_gbuffer.get(),
                m_phase1.frustumClusterCullDrawAlphaClipped,
                m_phase1.alphaClipIndexDraw,
                m_drawDataLine[dataline.alphaClipDataLine],
                m_debugDepth,
                drawCamera,
                JitterOption::Enabled,
                AlphaClipOption::Enabled,
                m_currentRenderMode,
                BiasOption::Disabled,
                DepthTestOption::GreaterEqual,
                m_virtualResolution,
                m_previousCameraViewMatrix,
                m_previousCameraProjectionMatrix);

			m_renderGbuffer.render(
				cmd,
				*m_gbuffer.get(),
				m_phase1.frustumClusterCullDrawTerrain,
				m_phase1.terrainIndexDraw,
				m_drawDataLine[dataline.terrainDataLine],
				m_debugDepth,
				drawCamera,
				JitterOption::Enabled,
				AlphaClipOption::Disabled,
				m_currentRenderMode,
				BiasOption::Disabled,
				DepthTestOption::GreaterEqual,
				m_virtualResolution,
				m_previousCameraViewMatrix,
				m_previousCameraProjectionMatrix);
        }
    }

    TextureRTV ModelRenderer::getCurrentLightingTarget()
    {
        if (m_taaEnabled)
            return m_lightingTarget;
        else
            return m_fullResTargetFrame[m_currentFullResIndex];
    }

    TextureSRV ModelRenderer::getCurrentLightingTargetSRV()
    {
        if (m_taaEnabled)
            return m_lightingTargetSRV;
        else
            return m_fullResTargetFrameSRV[m_currentFullResIndex];
    }

    TextureUAV ModelRenderer::getCurrentLightingTargetUAV()
    {
        if (m_taaEnabled)
            return m_lightingTargetUAV;
        else
            return m_fullResTargetFrameUAV[m_currentFullResIndex];
    }

	void ModelRenderer::drawFirstPassForward(
        CommandList& cmd, 
        DataLineSetup dataline, 
        DepthPyramid& depthPyramid, 
        FlatScene& scene,
		TextureSRV shadowMap,
		BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
		LightData& lights,
		const Vector3f& probePosition, float probeRange)
	{
		CPU_MARKER(cmd.api(), "Opaque pass");
		GPU_MARKER(cmd, "Opaque pass");

		{
			CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
			GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDraw,
				m_indexDataLine,
				m_phase1.depthIndexDraw,
				m_drawDataLine[0]);

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDrawAlphaClipped,
				m_indexDataLine,
				m_phase1.alphaClipIndexDraw,
				m_drawDataLine[dataline.alphaClipDataLine]);

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDrawTerrain,
				m_indexDataLine,
				m_phase1.terrainIndexDraw,
				m_drawDataLine[dataline.terrainDataLine],
				600u);
		}

		{
			CPU_MARKER(cmd.api(), "Z-Prepass for opaque and alphaclipped");
			GPU_MARKER(cmd, "Z-Prepass for opaque and alphaclipped");

			m_renderDepth.render(
				cmd,
				m_phase1.frustumClusterCullDraw,
				m_phase1.depthIndexDraw,
				m_drawDataLine[0],
				depthPyramid.dsv(),
				scene.drawCamera(),
				JitterOption::Enabled,
				AlphaClipOption::Disabled,
				BiasOption::Disabled,
				m_virtualResolution);

			// render alphaclipped depth
			m_renderDepth.render(
				cmd,
				m_phase1.frustumClusterCullDrawAlphaClipped,
				m_phase1.alphaClipIndexDraw,
				m_drawDataLine[dataline.alphaClipDataLine],
				depthPyramid.dsv(),
                scene.drawCamera(),
				JitterOption::Enabled,
				AlphaClipOption::Enabled,
				BiasOption::Disabled,
				m_virtualResolution);

			if (m_terrainDebugMode != 1)
			{
				for (auto&& terrain : scene.terrains)
				{
					terrain->terrain().renderDepth(
						cmd,
						m_phase1.frustumClusterCullDrawTerrain,
						m_phase1.terrainIndexDraw,
						m_drawDataLine[dataline.terrainDataLine],
						depthPyramid.dsv(),
						scene.drawCamera(),
						JitterOption::Enabled,
						BiasOption::Disabled,
						m_virtualResolution);
				}
			}
		}
        createDepthPyramid(cmd, depthPyramid);

        renderVoxels(cmd, scene, shadowMap, shadowVP, lightIndexToShadowIndex, lights);

        renderLightBins(m_device, cmd, depthPyramid, scene, *scene.lightData);

		// should draw ssao here
		renderSSAOForward(m_device, cmd, depthPyramid, scene);

		{
			CPU_MARKER(cmd.api(), "Forward lighting, depth equal pass for opaque and alphaclipped");
			GPU_MARKER(cmd, "Forward lighting, depth equal pass for opaque and alphaclipped");

			m_renderForward.render(
				cmd,
				m_phase1.frustumClusterCullDraw,
				m_phase1.depthIndexDraw,
				m_drawDataLine[0],
				depthPyramid.dsv(),
                getCurrentLightingTarget(),
				m_gbuffer->rtv(GBufferType::Motion),
				shadowMap,
				shadowVP,
				lightIndexToShadowIndex,
				m_blurTargetSRV,
                scene.drawCamera(),
				probePosition,
				probeRange,
				JitterOption::Enabled,
				AlphaClipOption::Disabled,
				DepthTestOption::Equal,
				static_cast<uint32_t>(m_currentRenderMode),
				m_virtualResolution,
				lights,
				m_shapeRenderer.lightBins(),
				m_previousCameraViewMatrix,
				m_previousCameraProjectionMatrix);

			m_renderForward.render(
				cmd,
				m_phase1.frustumClusterCullDrawAlphaClipped,
				m_phase1.alphaClipIndexDraw,
				m_drawDataLine[dataline.alphaClipDataLine],
				depthPyramid.dsv(),
                getCurrentLightingTarget(),
				m_gbuffer->rtv(GBufferType::Motion),
				shadowMap,
				shadowVP,
				lightIndexToShadowIndex,
				m_blurTargetSRV,
                scene.drawCamera(),
				probePosition,
				probeRange,
				JitterOption::Enabled,
				AlphaClipOption::Enabled,
				DepthTestOption::Equal,
				static_cast<uint32_t>(m_currentRenderMode),
				m_virtualResolution,
				lights,
				m_shapeRenderer.lightBins(),
				m_previousCameraViewMatrix,
				m_previousCameraProjectionMatrix);

			for (auto&& terrain : scene.terrains)
			{
				terrain->terrain().renderForward(
					cmd,
					m_phase1.frustumClusterCullDrawTerrain,
					m_phase1.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLine],
					depthPyramid.dsv(),
                    getCurrentLightingTarget(),
					m_gbuffer->rtv(GBufferType::Motion),
					shadowMap,
					shadowVP,
					lightIndexToShadowIndex,
					m_blurTargetSRV,
					scene.drawCamera(),
					probePosition,
					probeRange,
					JitterOption::Enabled,
					DepthTestOption::Equal,
					static_cast<uint32_t>(m_currentRenderMode),
					m_virtualResolution,
					lights,
					m_shapeRenderer.lightBins(),
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix,
					m_terrainDebugMode);
			}

			m_renderDxr.render(
				cmd,
				m_phase1.frustumClusterCullDraw,
				m_phase1.depthIndexDraw,
				m_drawDataLine[0],
				depthPyramid.dsv(),
                getCurrentLightingTarget(),
				m_gbuffer->rtv(GBufferType::Motion),
				shadowMap,
				shadowVP,
				lightIndexToShadowIndex,
				m_blurTargetSRV,
				scene.drawCamera(),
				probePosition,
				probeRange,
				JitterOption::Enabled,
				AlphaClipOption::Disabled,
				DepthTestOption::Equal,
				static_cast<uint32_t>(m_currentRenderMode),
				m_virtualResolution,
				lights,
				m_shapeRenderer.lightBins(),
				m_previousCameraViewMatrix,
				m_previousCameraProjectionMatrix);
		}
	}

    void ModelRenderer::drawFirstPassForwardDebug(
        CommandList& cmd, 
        DataLineSetup dataline, 
        DepthPyramid& depthPyramid, 
        FlatScene& scene,
        TextureSRV shadowMap,
        BufferSRV shadowVP,
        BufferSRV lightIndexToShadowIndex,
        LightData& lights,
        const Vector3f& probePosition, float probeRange)
    {
        CPU_MARKER(cmd.api(), "Opaque pass");
        GPU_MARKER(cmd, "Opaque pass");

        {
            CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
            GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");

            m_indexExpansion.expandIndexes(m_device, cmd,
                m_phase1.frustumClusterCullDraw,
                m_indexDataLine,
                m_phase1.depthIndexDraw,
                m_drawDataLine[0]);

            m_indexExpansion.expandIndexes(m_device, cmd,
                m_phase1.frustumClusterCullDrawAlphaClipped,
                m_indexDataLine,
                m_phase1.alphaClipIndexDraw,
                m_drawDataLine[dataline.alphaClipDataLine]);

			m_indexExpansion.expandIndexes(m_device, cmd,
				m_phase1.frustumClusterCullDrawTerrain,
				m_indexDataLine,
				m_phase1.terrainIndexDraw,
				m_drawDataLine[dataline.terrainDataLine],
				600u);
        }

        {
            CPU_MARKER(cmd.api(), "Write depth for opaque and alphaclipped");
            GPU_MARKER(cmd, "Write depth for opaque and alphaclipped");

            m_renderDepth.render(
                cmd,
                m_phase1.frustumClusterCullDraw,
                m_phase1.depthIndexDraw,
                m_drawDataLine[0],
                depthPyramid.dsv(),
                scene.cullingCamera(),
                JitterOption::Enabled,
                AlphaClipOption::Disabled,
                BiasOption::Disabled,
                m_virtualResolution);

            // render alphaclipped depth
            m_renderDepth.render(
                cmd,
                m_phase1.frustumClusterCullDrawAlphaClipped,
                m_phase1.alphaClipIndexDraw,
                m_drawDataLine[dataline.alphaClipDataLine],
                depthPyramid.dsv(),
                scene.cullingCamera(),
                JitterOption::Enabled,
                AlphaClipOption::Enabled,
                BiasOption::Disabled,
                m_virtualResolution);

			if (m_terrainDebugMode != 1)
			{
				for (auto&& terrain : scene.terrains)
				{
					terrain->terrain().renderDepth(
						cmd,
						m_phase1.frustumClusterCullDrawTerrain,
						m_phase1.terrainIndexDraw,
						m_drawDataLine[dataline.terrainDataLine],
						depthPyramid.dsv(),
						scene.cullingCamera(),
						JitterOption::Enabled,
						BiasOption::Disabled,
						m_virtualResolution);
				}
			}
        }

        createDepthPyramid(cmd, depthPyramid);

        if (!m_debugDepth)
        {
            m_debugDepth = m_device.createTextureDSV(TextureDescription()
                .name("Depth Buffer")
                .format(Format::D32_FLOAT)
                .width(depthPyramid.dsv().width())
                .height(depthPyramid.dsv().height())
                .usage(ResourceUsage::DepthStencil)
                .optimizedDepthClearValue(0.0f)
                .dimension(ResourceDimension::Texture2D)
            );
            m_debugDepthSRV = m_device.createTextureSRV(m_debugDepth);
        }

        {
            CPU_MARKER(cmd.api(), "Clear debug depth");
            GPU_MARKER(cmd, "Clear debug depth");
            cmd.clearDepthStencilView(m_debugDepth, 0.0f);
        }

        {
            CPU_MARKER(cmd.api(), "Write depth for opaque and alphaclipped");
            GPU_MARKER(cmd, "Write depth for opaque and alphaclipped");

            m_renderDepth.render(
                cmd,
                m_phase1.frustumClusterCullDraw,
                m_phase1.depthIndexDraw,
                m_drawDataLine[0],
                m_debugDepth,
                scene.drawCamera(),
                JitterOption::Enabled,
                AlphaClipOption::Disabled,
                BiasOption::Disabled,
                m_virtualResolution);

            // render alphaclipped depth
            m_renderDepth.render(
                cmd,
                m_phase1.frustumClusterCullDrawAlphaClipped,
                m_phase1.alphaClipIndexDraw,
                m_drawDataLine[dataline.alphaClipDataLine],
                m_debugDepth,
                scene.drawCamera(),
                JitterOption::Enabled,
                AlphaClipOption::Enabled,
                BiasOption::Disabled,
                m_virtualResolution);

			if (m_terrainDebugMode != 1)
			{
				for (auto&& terrain : scene.terrains)
				{
					terrain->terrain().renderDepth(
						cmd,
						m_phase1.frustumClusterCullDrawTerrain,
						m_phase1.terrainIndexDraw,
						m_drawDataLine[dataline.terrainDataLine],
						m_debugDepth,
						scene.drawCamera(),
						JitterOption::Enabled,
						BiasOption::Disabled,
						m_virtualResolution);
				}
			}
        }

        // should draw ssao here
        renderSSAOForward(m_device, cmd, depthPyramid, scene);

        {
            CPU_MARKER(cmd.api(), "Forward lighting, depth equal pass for opaque and alphaclipped");
            GPU_MARKER(cmd, "Forward lighting, depth equal pass for opaque and alphaclipped");

            m_renderForward.render(
                cmd,
                m_phase1.frustumClusterCullDraw,
                m_phase1.depthIndexDraw,
                m_drawDataLine[0],
                m_debugDepth,
                getCurrentLightingTarget(),
                m_gbuffer->rtv(GBufferType::Motion),
                shadowMap,
                shadowVP,
                lightIndexToShadowIndex,
                m_blurTargetSRV,
                scene.drawCamera(),
                probePosition,
                probeRange,
                JitterOption::Enabled,
                AlphaClipOption::Disabled,
                DepthTestOption::Equal,
                static_cast<uint32_t>(m_currentRenderMode),
                m_virtualResolution,
                lights,
                m_shapeRenderer.lightBins(),
                m_previousCameraViewMatrix,
                m_previousCameraProjectionMatrix);

            m_renderForward.render(
                cmd,
                m_phase1.frustumClusterCullDrawAlphaClipped,
                m_phase1.alphaClipIndexDraw,
                m_drawDataLine[dataline.alphaClipDataLine],
                m_debugDepth,
                getCurrentLightingTarget(),
                m_gbuffer->rtv(GBufferType::Motion),
                shadowMap,
                shadowVP,
                lightIndexToShadowIndex,
                m_blurTargetSRV,
                scene.drawCamera(),
                probePosition,
                probeRange,
                JitterOption::Enabled,
                AlphaClipOption::Enabled,
                DepthTestOption::Equal,
                static_cast<uint32_t>(m_currentRenderMode),
                m_virtualResolution,
                lights,
                m_shapeRenderer.lightBins(),
                m_previousCameraViewMatrix,
                m_previousCameraProjectionMatrix);

			for (auto&& terrain : scene.terrains)
			{
				terrain->terrain().renderForward(
					cmd,
					m_phase1.frustumClusterCullDrawTerrain,
					m_phase1.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLine],
					m_debugDepth,
                    getCurrentLightingTarget(),
					m_gbuffer->rtv(GBufferType::Motion),
					shadowMap,
					shadowVP,
					lightIndexToShadowIndex,
					m_blurTargetSRV,
					scene.drawCamera(),
					probePosition,
					probeRange,
					JitterOption::Enabled,
					DepthTestOption::Equal,
					static_cast<uint32_t>(m_currentRenderMode),
					m_virtualResolution,
					lights,
					m_shapeRenderer.lightBins(),
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix,
					m_terrainDebugMode);
			}
        }
    }

	void ModelRenderer::createDepthPyramid(CommandList& cmd, DepthPyramid& depthPyramid)
	{
		// 4. Depth downsample
		CPU_MARKER(cmd.api(), "4. Depth downsample");
		GPU_MARKER(cmd, "4. Depth downsample");
		depthPyramid.performDownsample(cmd);
	}

	void ModelRenderer::copyFirstPassStatistics(CommandList& cmd, DataLineSetup dataline)
	{
		// copy statistics
        // phase 1 (prev frame)
        CPU_MARKER(cmd.api(), "Phase 1 statistics");
        GPU_MARKER(cmd, "Phase 1 statistics");
        // clusterFrustumCull_ClustersIn
        cmd.copyBuffer(m_phase2.occlusionCullDrawAll.clusterCount.buffer, m_frameStatistics, 1, 0, 0); // instances out

        // clusterFrustumCull_ClustersOutGBuffer
        cmd.copyBuffer(m_phase1.frustumClusterCullDraw.clusterCount.buffer, m_frameStatistics, 1, 0, 1); // instances out

        // clusterFrustumCull_ClustersOutAlphaClipped
        cmd.copyBuffer(m_phase1.frustumClusterCullDrawAlphaClipped.clusterCount.buffer, m_frameStatistics, 1, 0, 2); // instances out

		// clusterFrustumCull_ClustersOutTransparent
		cmd.copyBuffer(m_phase1.frustumClusterCullDrawTransparent.clusterCount.buffer, m_frameStatistics, 1, 0, 3); // instances out

		// clusterFrustumCull_ClustersOutTerrain
		cmd.copyBuffer(m_phase1.frustumClusterCullDrawTerrain.clusterCount.buffer, m_frameStatistics, 1, 0, 4); // instances out



        // expandGBufferIndexes_IndexesOut
        cmd.copyBuffer(m_phase1.depthIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 5); // instances out

        // drawDepth_Triangles
        //cmd.copyBuffer(m_phase1.depthIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 6); // instances out

        // drawDepth_Count
        cmd.copyBuffer(m_drawDataLine[0].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0, 7);


        // expandAlphaClipIndexes_IndexesOut
        cmd.copyBuffer(m_phase1.alphaClipIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 8); // instances out

        // drawAlphaClip_Triangles
        //dc.load(cmd, m_frameStatistics, DataLinePoint::RecordType::Index, DataLinePoint::RecordBufferType::Count, DataLinePoint::Start, 9);

        // drawAlphaClip_Count
        cmd.copyBuffer(m_drawDataLine[dataline.alphaClipDataLine].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0, 10);


		// expandTransparentIndexes_IndexesOut
		cmd.copyBuffer(m_phase1.transparentIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 11); // instances out

		// drawTransparent_Triangles
		//cmd.copyBuffer(m_phase1.transparentIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 12); // instances out

		// drawTransparent_Count
		cmd.copyBuffer(m_drawDataLine[dataline.transparentDataLine].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0, 13);



		// expandTerrainIndexes_IndexesOut
		cmd.copyBuffer(m_phase1.terrainIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 14); // instances out

		// drawTerrain_Triangles
		//cmd.copyBuffer(m_phase1.terrainIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 15); // instances out

		// drawTerrain_Count
		cmd.copyBuffer(m_drawDataLine[dataline.terrainDataLine].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0, 16);

	}

	void ModelRenderer::frustumInstanceCull(CommandList& cmd, DataLineSetup dataline, DepthPyramid& depthPyramid, Camera& cullingCamera)
	{
		CPU_MARKER(cmd.api(), "5. Instance Frustum (increases cluster count)");
		GPU_MARKER(cmd, "5. Instance Frustum (increases cluster count)");
		m_frustumCuller.instanceCull(
			cmd, cullingCamera, m_device.modelResources(), m_virtualResolution, &depthPyramid,
			m_clusterDataLine[dataline.currentDataLine],
			m_phase2.frustumInstanceCullDraw);
	}

	void ModelRenderer::expandClusters(CommandList& cmd)
	{
		CPU_MARKER(cmd.api(), "6. Cluster expansion (adds new clusters)");
		GPU_MARKER(cmd, "6. Cluster expansion (adds new clusters)");
		m_frustumCuller.expandClusters(cmd, m_phase2.frustumInstanceCullDraw);
	}

	void ModelRenderer::occlusionCull(CommandList& cmd, DataLineSetup dataline, DepthPyramid& depthPyramid, Camera& cullingCamera)
	{
		CPU_MARKER(cmd.api(), "7. Occlusion culling and cluster cone culling");
		GPU_MARKER(cmd, "7. Occlusion culling and cluster cone culling");

		m_clusterDataLine[dataline.previousDataLine].reset(cmd);
		m_clusterDataLine[dataline.alphaClipDataLine].reset(cmd);
		m_clusterDataLine[dataline.transparentDataLine].reset(cmd);
		m_clusterDataLine[dataline.terrainDataLine].reset(cmd);

		m_occlusionCuller.occlusionCull(
			cmd, cullingCamera, m_virtualResolution, &depthPyramid,

			m_phase2.frustumInstanceCullDraw,       // input
			m_clusterDataLine[dataline.currentDataLine],     // output all
			m_clusterDataLine[dataline.previousDataLine],    // output not yet drawn depth
			m_clusterDataLine[dataline.alphaClipDataLine],   // output alphaclipped
			m_clusterDataLine[dataline.transparentDataLine],   // output alphaclipped
			m_clusterDataLine[dataline.terrainDataLine],   // output alphaclipped

			m_phase2.occlusionCullDrawAll,
			m_phase2.occlusionCullDrawNotYetDrawnDepth,
			m_phase2.occlusionCullDrawAlphaClipped,
			m_phase2.occlusionCullDrawTransparent,
			m_phase2.occlusionCullDrawTerrain,

            m_device.modelResources().gpuBuffers().instanceClusterTracking(),
            m_device.modelResources().gpuBuffers().clusterTrackingClusterInstancesUAV);
	}

	void ModelRenderer::drawSecondPassDeferred(
		CommandList& cmd, 
		DataLineSetup dataline, 
		DepthPyramid& depthPyramid, 
		Camera& drawCamera)
	{
		CPU_MARKER(cmd.api(), "Opaque pass (2nd)");
		GPU_MARKER(cmd, "Opaque pass (2nd)");
		{
			{
				CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
				GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");

				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawNotYetDrawnDepth,
					m_indexDataLine,
					m_phase2.notYetDrawnDepthIndexDraw,
					m_drawDataLine[1]);

				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawAlphaClipped,
					m_indexDataLine,
					m_phase2.alphaClipIndexDraw,
					m_drawDataLine[dataline.alphaClipDataLineSecond]);

				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawTerrain,
					m_indexDataLine,
					m_phase2.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLineSecond],
					600u);
			}

			{
				CPU_MARKER(cmd.api(), "GBuffer pass for opaque and alphaclipped");
				GPU_MARKER(cmd, "GBuffer pass for opaque and alphaclipped");

				m_renderGbuffer.render(
					cmd,
					*m_gbuffer.get(),
					m_clusterDataLine[dataline.previousDataLine],
					m_phase2.notYetDrawnDepthIndexDraw,
					m_drawDataLine[1],
					depthPyramid.dsv(),
					drawCamera,
					JitterOption::Enabled,
					AlphaClipOption::Disabled,
                    m_currentRenderMode,
					BiasOption::Disabled,
					DepthTestOption::GreaterEqual,
					m_virtualResolution,
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix);

				m_renderGbuffer.render(
					cmd,
					*m_gbuffer.get(),
					m_phase2.occlusionCullDrawAlphaClipped,
					m_phase2.alphaClipIndexDraw,
					m_drawDataLine[dataline.alphaClipDataLineSecond],
					depthPyramid.dsv(),
					drawCamera,
					JitterOption::Enabled,
					AlphaClipOption::Enabled,
                    m_currentRenderMode,
					BiasOption::Disabled,
					DepthTestOption::GreaterEqual,
					m_virtualResolution,
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix);

				m_renderGbuffer.render(
					cmd,
					*m_gbuffer.get(),
					m_phase2.occlusionCullDrawTerrain,
					m_phase2.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLineSecond],
					depthPyramid.dsv(),
					drawCamera,
					JitterOption::Enabled,
					AlphaClipOption::Disabled,
					m_currentRenderMode,
					BiasOption::Disabled,
					DepthTestOption::GreaterEqual,
					m_virtualResolution,
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix);
			}
		}
	}

    void ModelRenderer::drawSecondPassDeferredDebug(
        CommandList& cmd,
        DataLineSetup dataline,
        Camera& drawCamera)
    {
        CPU_MARKER(cmd.api(), "Opaque pass (2nd)");
        GPU_MARKER(cmd, "Opaque pass (2nd)");
        {
            {
                CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
                GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");

                m_indexExpansion.expandIndexes(m_device, cmd,
                    m_phase2.occlusionCullDrawNotYetDrawnDepth,
                    m_indexDataLine,
                    m_phase2.notYetDrawnDepthIndexDraw,
                    m_drawDataLine[1]);

                m_indexExpansion.expandIndexes(m_device, cmd,
                    m_phase2.occlusionCullDrawAlphaClipped,
                    m_indexDataLine,
                    m_phase2.alphaClipIndexDraw,
                    m_drawDataLine[dataline.alphaClipDataLineSecond]);

				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawTerrain,
					m_indexDataLine,
					m_phase2.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLineSecond],
					600u);
            }

            {
                CPU_MARKER(cmd.api(), "GBuffer pass for opaque and alphaclipped");
                GPU_MARKER(cmd, "GBuffer pass for opaque and alphaclipped");

                m_renderGbuffer.render(
                    cmd,
                    *m_gbuffer.get(),
                    m_clusterDataLine[dataline.previousDataLine],
                    m_phase2.notYetDrawnDepthIndexDraw,
                    m_drawDataLine[1],
                    m_debugDepth,
                    drawCamera,
                    JitterOption::Enabled,
                    AlphaClipOption::Disabled,
                    m_currentRenderMode,
                    BiasOption::Disabled,
                    DepthTestOption::GreaterEqual,
                    m_virtualResolution,
                    m_previousCameraViewMatrix,
                    m_previousCameraProjectionMatrix);

                m_renderGbuffer.render(
                    cmd,
                    *m_gbuffer.get(),
                    m_phase2.occlusionCullDrawAlphaClipped,
                    m_phase2.alphaClipIndexDraw,
                    m_drawDataLine[dataline.alphaClipDataLineSecond],
                    m_debugDepth,
                    drawCamera,
                    JitterOption::Enabled,
                    AlphaClipOption::Enabled,
                    m_currentRenderMode,
                    BiasOption::Disabled,
                    DepthTestOption::GreaterEqual,
                    m_virtualResolution,
                    m_previousCameraViewMatrix,
                    m_previousCameraProjectionMatrix);

				m_renderGbuffer.render(
					cmd,
					*m_gbuffer.get(),
					m_phase2.occlusionCullDrawTerrain,
					m_phase2.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLineSecond],
					m_debugDepth,
					drawCamera,
					JitterOption::Enabled,
					AlphaClipOption::Disabled,
					m_currentRenderMode,
					BiasOption::Disabled,
					DepthTestOption::GreaterEqual,
					m_virtualResolution,
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix);
            }
        }
    }

	void ModelRenderer::drawSecondPassForward(
		CommandList& cmd, 
		DataLineSetup dataline, 
		DepthPyramid& depthPyramid, 
		Camera& drawCamera,
		TextureSRV shadowMap,
		BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
		LightData& lights,
		const Vector3f& probePosition,
		float probeRange,
		FlatScene& scene)
	{
		CPU_MARKER(cmd.api(), "Opaque pass (2nd)");
		GPU_MARKER(cmd, "Opaque pass (2nd)");
		{
			{
				CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
				GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");
				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawNotYetDrawnDepth,
					m_indexDataLine,
					m_phase2.notYetDrawnDepthIndexDraw,
					m_drawDataLine[1]);

				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawAlphaClipped,
					m_indexDataLine,
					m_phase2.alphaClipIndexDraw,
					m_drawDataLine[dataline.alphaClipDataLineSecond]);

				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawTerrain,
					m_indexDataLine,
					m_phase2.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLineSecond],
					600u);
			}

			{
				CPU_MARKER(cmd.api(), "Z-Prepass for opaque and alphaclipped");
				GPU_MARKER(cmd, "Z-Prepass for opaque and alphaclipped");

				m_renderDepth.render(
					cmd,
					m_clusterDataLine[dataline.previousDataLine],
					m_phase2.notYetDrawnDepthIndexDraw,
					m_drawDataLine[1],
					depthPyramid.dsv(),
					drawCamera,
					JitterOption::Enabled,
					AlphaClipOption::Disabled,
					BiasOption::Disabled,
					m_virtualResolution);

				// render alphaclipped depth
				m_renderDepth.render(
					cmd,
					m_phase2.occlusionCullDrawAlphaClipped,
					m_phase2.alphaClipIndexDraw,
					m_drawDataLine[dataline.alphaClipDataLineSecond],
					depthPyramid.dsv(),
					drawCamera,
					JitterOption::Enabled,
					AlphaClipOption::Enabled,
					BiasOption::Disabled,
					m_virtualResolution);

				// render terrain depth
				if (m_terrainDebugMode != 1)
				{
					for (auto&& terrain : scene.terrains)
					{
						terrain->terrain().renderDepth(
							cmd,
							m_phase2.occlusionCullDrawTerrain,
							m_phase2.terrainIndexDraw,
							m_drawDataLine[dataline.terrainDataLineSecond],
							depthPyramid.dsv(),
							drawCamera,
							JitterOption::Enabled,
							BiasOption::Disabled,
							m_virtualResolution);
					}
				}
			}

			{
				CPU_MARKER(cmd.api(), "Forward lighting, depth equal pass for opaque and alphaclipped");
				GPU_MARKER(cmd, "Forward lighting, depth equal pass for opaque and alphaclipped");

				m_renderForward.render(
					cmd,
					m_clusterDataLine[dataline.previousDataLine],
					m_phase2.notYetDrawnDepthIndexDraw,
					m_drawDataLine[1],
					depthPyramid.dsv(),
                    getCurrentLightingTarget(),
					m_gbuffer->rtv(GBufferType::Motion),
					shadowMap,
					shadowVP,
					lightIndexToShadowIndex,
					m_blurTargetSRV,
					drawCamera,
					probePosition,
					probeRange,
					JitterOption::Enabled,
					AlphaClipOption::Disabled,
					DepthTestOption::Equal,
					static_cast<uint32_t>(m_currentRenderMode),
					m_virtualResolution,
					lights,
					m_shapeRenderer.lightBins(),
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix);

				m_renderForward.render(
					cmd,
					m_phase2.occlusionCullDrawAlphaClipped,
					m_phase2.alphaClipIndexDraw,
					m_drawDataLine[dataline.alphaClipDataLineSecond],
					depthPyramid.dsv(),
                    getCurrentLightingTarget(),
					m_gbuffer->rtv(GBufferType::Motion),
					shadowMap,
					shadowVP,
					lightIndexToShadowIndex,
					m_blurTargetSRV,
					drawCamera,
					probePosition,
					probeRange,
					JitterOption::Enabled,
					AlphaClipOption::Enabled,
					DepthTestOption::Equal,
					static_cast<uint32_t>(m_currentRenderMode),
					m_virtualResolution,
					lights,
					m_shapeRenderer.lightBins(),
					m_previousCameraViewMatrix,
					m_previousCameraProjectionMatrix);

				for (auto&& terrain : scene.terrains)
				{
					terrain->terrain().renderForward(
						cmd,
						m_phase2.occlusionCullDrawTerrain,
						m_phase2.terrainIndexDraw,
						m_drawDataLine[dataline.terrainDataLineSecond],
						depthPyramid.dsv(),
                        getCurrentLightingTarget(),
						m_gbuffer->rtv(GBufferType::Motion),
						shadowMap,
						shadowVP,
						lightIndexToShadowIndex,
						m_blurTargetSRV,
						drawCamera,
						probePosition,
						probeRange,
						JitterOption::Enabled,
						DepthTestOption::Equal,
						static_cast<uint32_t>(m_currentRenderMode),
						m_virtualResolution,
						lights,
						m_shapeRenderer.lightBins(),
						m_previousCameraViewMatrix,
						m_previousCameraProjectionMatrix,
						m_terrainDebugMode);
				}
			}
		}

#ifdef ECS_TEST_ENABLED
		m_ecsDemo.render(cmd, getCurrentLightingTarget(), depthPyramid.dsv(), drawCamera, m_jitterMatrix);
#endif
	}

    void ModelRenderer::drawSecondPassForwardDebug(
        CommandList& cmd,
        DataLineSetup dataline,
        DepthPyramid& depthPyramid,
        Camera& drawCamera,
        TextureSRV shadowMap,
        BufferSRV shadowVP,
        BufferSRV lightIndexToShadowIndex,
        LightData& lights,
        const Vector3f& probePosition,
        float probeRange,
		FlatScene& scene)
    {
        CPU_MARKER(cmd.api(), "Opaque pass (2nd)");
        GPU_MARKER(cmd, "Opaque pass (2nd)");
        {
            {
                CPU_MARKER(cmd.api(), "Expand opaque and alphaclipped indexes");
                GPU_MARKER(cmd, "Expand opaque and alphaclipped indexes");
                m_indexExpansion.expandIndexes(m_device, cmd,
                    m_phase2.occlusionCullDrawNotYetDrawnDepth,
                    m_indexDataLine,
                    m_phase2.notYetDrawnDepthIndexDraw,
                    m_drawDataLine[1]);

                m_indexExpansion.expandIndexes(m_device, cmd,
                    m_phase2.occlusionCullDrawAlphaClipped,
                    m_indexDataLine,
                    m_phase2.alphaClipIndexDraw,
                    m_drawDataLine[dataline.alphaClipDataLineSecond]);

				m_indexExpansion.expandIndexes(m_device, cmd,
					m_phase2.occlusionCullDrawTerrain,
					m_indexDataLine,
					m_phase2.terrainIndexDraw,
					m_drawDataLine[dataline.terrainDataLineSecond],
					600u);
            }

            {
                CPU_MARKER(cmd.api(), "Z-Prepass for opaque and alphaclipped");
                GPU_MARKER(cmd, "Z-Prepass for opaque and alphaclipped");

                m_renderDepth.render(
                    cmd,
                    m_clusterDataLine[dataline.previousDataLine],
                    m_phase2.notYetDrawnDepthIndexDraw,
                    m_drawDataLine[1],
                    m_debugDepth,
                    drawCamera,
                    JitterOption::Enabled,
                    AlphaClipOption::Disabled,
                    BiasOption::Disabled,
                    m_virtualResolution);

                // render alphaclipped depth
                m_renderDepth.render(
                    cmd,
                    m_phase2.occlusionCullDrawAlphaClipped,
                    m_phase2.alphaClipIndexDraw,
                    m_drawDataLine[dataline.alphaClipDataLineSecond],
                    m_debugDepth,
                    drawCamera,
                    JitterOption::Enabled,
                    AlphaClipOption::Enabled,
                    BiasOption::Disabled,
                    m_virtualResolution);

				// render terrain depth
				if (m_terrainDebugMode != 1)
				{
					for (auto&& terrain : scene.terrains)
					{
						terrain->terrain().renderDepth(
							cmd,
							m_phase2.occlusionCullDrawTerrain,
							m_phase2.terrainIndexDraw,
							m_drawDataLine[dataline.terrainDataLineSecond],
							m_debugDepth,
							drawCamera,
							JitterOption::Enabled,
							BiasOption::Disabled,
							m_virtualResolution);
					}
				}
            }

            {
                CPU_MARKER(cmd.api(), "Forward lighting, depth equal pass for opaque and alphaclipped");
                GPU_MARKER(cmd, "Forward lighting, depth equal pass for opaque and alphaclipped");

                m_renderForward.render(
                    cmd,
                    m_clusterDataLine[dataline.previousDataLine],
                    m_phase2.notYetDrawnDepthIndexDraw,
                    m_drawDataLine[1],
                    m_debugDepth,
                    getCurrentLightingTarget(),
                    m_gbuffer->rtv(GBufferType::Motion),
                    shadowMap,
                    shadowVP,
                    lightIndexToShadowIndex,
                    m_blurTargetSRV,
                    drawCamera,
                    probePosition,
                    probeRange,
                    JitterOption::Enabled,
                    AlphaClipOption::Disabled,
                    DepthTestOption::Equal,
                    static_cast<uint32_t>(m_currentRenderMode),
                    m_virtualResolution,
                    lights,
                    m_shapeRenderer.lightBins(),
                    m_previousCameraViewMatrix,
                    m_previousCameraProjectionMatrix);

                m_renderForward.render(
                    cmd,
                    m_phase2.occlusionCullDrawAlphaClipped,
                    m_phase2.alphaClipIndexDraw,
                    m_drawDataLine[dataline.alphaClipDataLineSecond],
                    m_debugDepth,
                    getCurrentLightingTarget(),
                    m_gbuffer->rtv(GBufferType::Motion),
                    shadowMap,
                    shadowVP,
                    lightIndexToShadowIndex,
                    m_blurTargetSRV,
                    drawCamera,
                    probePosition,
                    probeRange,
                    JitterOption::Enabled,
                    AlphaClipOption::Enabled,
                    DepthTestOption::Equal,
                    static_cast<uint32_t>(m_currentRenderMode),
                    m_virtualResolution,
                    lights,
                    m_shapeRenderer.lightBins(),
                    m_previousCameraViewMatrix,
                    m_previousCameraProjectionMatrix);

				for (auto&& terrain : scene.terrains)
				{
					terrain->terrain().renderForward(
						cmd,
						m_phase2.occlusionCullDrawTerrain,
						m_phase2.terrainIndexDraw,
						m_drawDataLine[dataline.terrainDataLineSecond],
						m_debugDepth,
                        getCurrentLightingTarget(),
						m_gbuffer->rtv(GBufferType::Motion),
						shadowMap,
						shadowVP,
						lightIndexToShadowIndex,
						m_blurTargetSRV,
						drawCamera,
						probePosition,
						probeRange,
						JitterOption::Enabled,
						DepthTestOption::Equal,
						static_cast<uint32_t>(m_currentRenderMode),
						m_virtualResolution,
						lights,
						m_shapeRenderer.lightBins(),
						m_previousCameraViewMatrix,
						m_previousCameraProjectionMatrix,
						m_terrainDebugMode);
				}
            }
        }

#ifdef ECS_TEST_ENABLED
        m_ecsDemo.render(cmd, getCurrentLightingTarget(), depthPyramid.dsv(), drawCamera, m_jitterMatrix);
#endif
    }

	void ModelRenderer::copySecondPassStatistics(CommandList& cmd, DataLineSetup dataline)
	{
		CPU_MARKER(cmd.api(), "Phase 2 statistics");
        GPU_MARKER(cmd, "Phase 2 statistics");

        /*// expandTransparentIndexes_IndexesOut
        cmd.copyBuffer(m_phase1.transparentIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0, 17); // instances out

        // drawTransparent_Triangles
        //dc.load(cmd, m_frameStatistics, DataLinePoint::RecordType::Index, DataLinePoint::RecordBufferType::Count, DataLinePoint::Start, 10);

        // drawTransparent_Count
        cmd.copyBuffer(m_drawDataLine[4].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0, 11);*/


        // instanceFrustum_InstancesIn
        //dc.load(cmd, m_frameStatistics, DataLinePoint::RecordType::Cluster, DataLinePoint::RecordBufferType::Count, DataLinePoint::p1, 17);

        // instanceFrustum_InstancesPassed
        cmd.copyBuffer(m_frustumCuller.instanceCount().buffer(), m_frameStatistics, 1, 0,                                               18); // instances out

        // clusterExpansion_ClustersOut
        cmd.copyBuffer(m_phase2.frustumInstanceCullDraw.clusterCount.buffer, m_frameStatistics, 1, 0,                                   19);


        // occlusionCulling_out_All
        cmd.copyBuffer(m_phase2.occlusionCullDrawAll.clusterCount.buffer, m_frameStatistics, 1, 0,                                      20);

        // occlusionCulling_out_NotYetDrawnDepth
        cmd.copyBuffer(m_phase2.occlusionCullDrawNotYetDrawnDepth.clusterCount.buffer, m_frameStatistics, 1, 0,                         21);

        // occlusionCulling_out_AlphaClipped
        cmd.copyBuffer(m_phase2.occlusionCullDrawAlphaClipped.clusterCount.buffer, m_frameStatistics, 1, 0,                             22);

        // occlusionCulling_out_Transparent
        cmd.copyBuffer(m_phase2.occlusionCullDrawTransparent.clusterCount.buffer, m_frameStatistics, 1, 0,                              23);

		// occlusionCulling_out_Terrain
		cmd.copyBuffer(m_phase2.occlusionCullDrawTerrain.clusterCount.buffer, m_frameStatistics, 1, 0,									24);


        // expandIndexes_NotYetDrawnDepthIndexesOut
        cmd.copyBuffer(m_phase2.notYetDrawnDepthIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,                                   25);
            
        // drawDepth_Triangles2 = expandIndexes_NotYetDrawnDepthIndexesOut / 3
        //cmd.copyBuffer(m_phase2.notYetDrawnDepthIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,                                 26);

        // drawDepth_Count2
        cmd.copyBuffer(m_drawDataLine[1].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0,                           27);


        // expandIndexes_AlphaClippedIndexesOut
        cmd.copyBuffer(m_phase2.alphaClipIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,                                          28);
            
        // drawDepth_AlphaClippedTriangles
        //cmd.copyBuffer(m_phase2.notYetDrawnDepthIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,                                 29);

        // drawDepth_AlphaClippedCount
        cmd.copyBuffer(m_drawDataLine[dataline.alphaClipDataLine].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0,       30);



        // expandIndexes_TransparentIndexesOut
        cmd.copyBuffer(m_phase2.transparentIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,                                        31);
            
        // drawDepth_TransparentTriangles
        //cmd.copyBuffer(m_phase2.notYetDrawnDepthIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,                                 32);

        // drawDepth_TransparentCount
        cmd.copyBuffer(m_drawDataLine[dataline.transparentDataLine].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0,                           33);



		// expandIndexes_TerrainIndexesOut
        cmd.copyBuffer(m_phase2.terrainIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,											34);
            
        // drawDepth_TerrainTriangles
        //cmd.copyBuffer(m_phase2.notYetDrawnDepthIndexDraw.indexCount.buffer, m_frameStatistics, 1, 0,                                 35);

        // drawDepth_TerrainCount
        cmd.copyBuffer(m_drawDataLine[dataline.terrainDataLine].clusterRendererExecuteIndirectCount.buffer, m_frameStatistics, 1, 0,    36);
	}

	void ModelRenderer::resetBloomFilter(CommandList& cmd)
	{
		CPU_MARKER(cmd.api(), "Reset Bloom filter");
		GPU_MARKER(cmd, "Reset Bloom filter");

		//cmd.clearBuffer(m_clusterTrackingUAV, 0xffffffff);

        if(m_device.modelResources().gpuBuffers().clusterTrackingClusterInstancesUAV)
            cmd.clearBuffer(m_device.modelResources().gpuBuffers().clusterTrackingClusterInstancesUAV);

		/*{
			m_clusterTrackingResetCreateArgumentsPipeline.cs.clusterTrackingCount = m_clusterTrackingResetCounter.srv;
			m_clusterTrackingResetCreateArgumentsPipeline.cs.clusterTrackingResetDispatchArgs = m_clusterTrackingResetDispatchArgsUAV;
			cmd.bindPipe(m_clusterTrackingResetCreateArgumentsPipeline);
			cmd.dispatch(1, 1, 1);
		}

		{
			m_clusterTrackingResetPipeline.cs.clusterTracking = clusterTrackingUAV();
			m_clusterTrackingResetPipeline.cs.clusterTrackingResetIndexes = clusterTrackingResetSRV();
			m_clusterTrackingResetPipeline.cs.clusterTrackingCount = m_clusterTrackingResetCounter.srv;
			cmd.bindPipe(m_clusterTrackingResetPipeline);
			cmd.dispatchIndirect(m_clusterTrackingResetDispatchArgs, 0);
		}*/
	}

	void ModelRenderer::renderLightBins(
		Device& device,
		CommandList& cmd,
		DepthPyramid& depthPyramid,
        FlatScene& scene,
		LightData& lights)
	{
		m_shapeRenderer.render(
			cmd, 
			depthPyramid, 
			scene.drawCamera(), 
			scene.drawCamera().jitterMatrix(device.frameNumber(), m_virtualResolution), 
			lights);
	}

    void ModelRenderer::renderPicker(
        CommandList& cmd, 
        unsigned int mouseX,
        unsigned int mouseY)
    {
        CPU_MARKER(cmd.api(), "Picker");
        GPU_MARKER(cmd, "Picker");
        m_picker.pick(m_gbuffer.get(), cmd, mouseX, mouseY);
    }

    void ModelRenderer::renderSSAO(
        Device& device, 
        CommandList& cmd, 
        DepthPyramid& depthPyramid,
        FlatScene& scene)
    {
        // SSAO
        if (m_ssaoEnabled)
        {
            if(scene.cameraDebugging())
                renderSSAO(device, cmd, m_debugDepthSRV, scene.drawCamera());
            else
                renderSSAO(device, cmd, depthPyramid.srv(), scene.drawCamera());
        }
        else
        {
            CPU_MARKER(cmd.api(), "Clear SSAO blur target");
            GPU_MARKER(cmd, "Clear SSAO blur target");
            cmd.clearRenderTargetView(m_blurTarget);
        }
    }

	void ModelRenderer::renderSSAOForward(
		Device& device,
		CommandList& cmd,
		DepthPyramid& depthPyramid,
        FlatScene& scene)
	{
		// SSAO
        if (m_ssaoEnabled)
        {
            if(scene.cameraDebugging())
                renderSSAOForward(device, cmd, m_debugDepthSRV, scene.drawCamera());
            else
                renderSSAOForward(device, cmd, depthPyramid.srv(), scene.drawCamera());
        }
		else
		{
			CPU_MARKER(cmd.api(), "Clear SSAO blur target");
			GPU_MARKER(cmd, "Clear SSAO blur target");
			if(m_blurTarget)
				cmd.clearRenderTargetView(m_blurTarget);
		}
	}

    void ModelRenderer::renderLighting(
        CommandList& cmd, 
        FlatScene& scene,
        DepthPyramid& depthPyramid,
        TextureSRV shadowMap,
        BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex)
    {
        //if (scene.lights.size() == 0)
        //    return;

        // lighting
        Vector3f probePosition;
        float probeRange = 100.0f;
        if (scene.probes.size() > 0)
        {
            ProbeComponent& probe = *scene.probes[0];
            probePosition = probe.position();
            probeRange = probe.range();
        }
        renderLighting(
            cmd, 
            depthPyramid,
            scene,
            shadowMap, shadowVP, lightIndexToShadowIndex,
            m_projectionMatrix, m_viewMatrix, 
            *scene.lightData, 
            probePosition, probeRange, 
            m_transparent.frameDownsampleChain());
    }

    void ModelRenderer::renderOutline(
        Device& device,
        CommandList& cmd,
        FlatScene& scene,
        DepthPyramid& depthPyramid,
        Camera& camera)
    {
        // render outline
        if (m_selectedObject != -1)
        {
            CPU_MARKER(cmd.api(), "Outline");
            GPU_MARKER(cmd, "Outline");

            for (auto& node : scene.nodes)
            {
                //if (node.objectId == m_selectedObject)
                if (node.mesh->meshBuffer().modelAllocations && node.mesh->meshBuffer().modelAllocations->subMeshInstance->instanceData.modelResource.gpuIndex == static_cast<size_t>(m_selectedObject))
                {
                    m_renderOutline->render(device,
                        getCurrentLightingTarget(),
                        depthPyramid.srv(),
                        camera,
                        device.modelResources(),
                        cmd,
                        node
                    );
                }
            }
            for (auto& node : scene.alphaclippedNodes)
            {
                //if (node.objectId == m_selectedObject)
                if (node.mesh->meshBuffer().modelAllocations && node.mesh->meshBuffer().modelAllocations->subMeshInstance->instanceData.modelResource.gpuIndex == static_cast<size_t>(m_selectedObject))
                {
                    m_renderOutline->render(device,
                        getCurrentLightingTarget(),
                        depthPyramid.srv(),
                        camera,
                        device.modelResources(),
                        cmd,
                        node
                    );
                }
            }
        }
    }

    void ModelRenderer::renderTemporalResolve(
        CommandList& cmd,
        DepthPyramid& depthPyramid,
        Camera& camera)
    {
        // remporal resolve
        if(m_taaEnabled)
            renderTemporalResolve(cmd, depthPyramid.srv(), camera, m_jitterValue, m_previousJitter, m_jitterMatrix);
    }

    void ModelRenderer::flip()
    {

        ++m_currentFullResIndex;
        if (m_currentFullResIndex >= HistoryCount)
            m_currentFullResIndex = 0;

        m_modelInstanceCount = m_device.modelResources().instanceCount();

        m_previousCameraViewMatrix = m_viewMatrix;
        m_previousCameraProjectionMatrix = m_projectionMatrix;
        m_previousJitterMatrix = m_jitterMatrix;
        m_previousJitter = m_jitterValue;

        m_currentDataLine = (m_currentDataLine + 1) % DataLineCount;
    }

    FrameStatistics ModelRenderer::getStatistics()
    {
        auto frameStat = reinterpret_cast<FrameStatistics*>(m_frameStatistics.resource().map(m_device));
        memcpy(&m_statistics, frameStat, sizeof(FrameStatistics));
        m_frameStatistics.resource().unmap(m_device);

        m_statistics.instanceFrustum_InstancesIn = m_modelInstanceCount;
        m_statistics.drawDepth_Triangles = m_statistics.expandGBufferIndexes_IndexesOut / 3;
        m_statistics.drawAlphaClip_Triangles = m_statistics.expandAlphaClipIndexes_IndexesOut / 3;
        m_statistics.drawTransparent_Triangles = m_statistics.expandTransparentIndexes_IndexesOut / 3;
		m_statistics.drawTerrain_Triangles = m_statistics.expandTerrainIndexes_IndexesOut / 3;
        m_statistics.drawDepth_Triangles2 = m_statistics.expandIndexes_NotYetDrawnDepthIndexesOut / 3;
        m_statistics.drawDepth_AlphaClippedTriangles = m_statistics.expandIndexes_AlphaClippedIndexesOut / 3;
        m_statistics.drawDepth_TransparentTriangles = m_statistics.expandIndexes_TransparentIndexesOut / 3;
		m_statistics.drawDepth_TerrainTriangles = m_statistics.expandIndexes_TerrainIndexesOut / 3;
        return m_statistics;
    }

    void ModelRenderer::renderGeometry(
        Device& /*device*/,
        TextureRTV /*currentRenderTarget*/,
        TextureDSV /*depthBuffer*/,
        CommandList& /*cmd*/,
        FlatScene& /*scene*/,
        Matrix4f /*cameraProjectionMatrix*/,
        Matrix4f /*cameraViewMatrix*/,
        Camera& /*camera*/,
        LightData& /*lights*/,
        TextureSRV /*shadowMap*/,
        BufferSRV /*shadowVP*/)
    {
    }

    unsigned int ModelRenderer::pickedObject(Device& device)
    {
        return m_picker.selectedInstanceId(device);
    }

	void ModelRenderer::setSSRDebugMousePosition(int x, int y)
	{
		m_ssrRenderer.setSSRDebugMousePosition(x, y);
	}

    void ModelRenderer::renderLighting(
        CommandList& cmd,
        DepthPyramid& depthPyramid,
        FlatScene& scene,
        TextureSRV shadowMap,
        BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
        Matrix4f cameraProjectionMatrix,
        Matrix4f cameraViewMatrix,
        LightData& lights,
        Vector3f probePosition,
        float probeRange,
        TextureSRV frameDownsampleChain)
    {
		{
			CPU_MARKER(cmd.api(), "Lighting");
			GPU_MARKER(cmd, "Lighting");

            auto& camera = scene.drawCamera();

			// lighting phase
			cmd.setRenderTargets({ getCurrentLightingTarget() });

			engine::Pipeline<shaders::Lighting>* pipe = nullptr;
            if (m_voxelize && m_sceneVoxelizer)
            {
                if (m_currentRenderMode == DEBUG_MODE_NONE)
                    pipe = &m_lightingPipelineVoxelReflections;
                else
                    pipe = &m_debugLightingPipelineVoxelReflections;
            }
            else
            {
                if (m_currentRenderMode == DEBUG_MODE_NONE)
                    pipe = &m_lightingPipeline;
                else
                    pipe = &m_debugLightingPipeline;
            }
			pipe->ps.debugMode.x = m_currentRenderMode;

			pipe->ps.cameraWorldSpacePosition = camera.position();
			pipe->ps.environmentStrength = camera.environmentMapStrength();
			if (camera.environmentIrradiance().valid() && camera.environmentIrradiance().texture().arraySlices() == 1)
			{
				pipe->ps.environmentIrradiance = camera.environmentIrradiance();
				pipe->ps.environmentIrradianceCubemap = TextureSRV();
				pipe->ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(false);
				pipe->ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(true);
			}
			else
			{
				pipe->ps.environmentIrradiance = TextureSRV();
				pipe->ps.environmentIrradianceCubemap = camera.environmentIrradiance();
				pipe->ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(true);
				pipe->ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(false);
			}
			pipe->ps.environmentSpecular = camera.environmentSpecular();
			pipe->ps.environmentBrdfLut = camera.environmentBrdfLUT();
			pipe->ps.hasEnvironmentSpecular.x = camera.environmentSpecular().valid();
			pipe->ps.shadowSize = Float2{ 1.0f / static_cast<float>(ShadowMapWidth), 1.0f / static_cast<float>(ShadowMapHeight) };
			pipe->ps.shadowMap = shadowMap;
			pipe->ps.shadowVP = shadowVP;
            if(scene.cameraDebugging())
			    pipe->ps.depth = m_debugDepthSRV;
            else
                pipe->ps.depth = depthPyramid.srv();
			if (frameDownsampleChain.valid())
			{
				//pipe->ps.frameBlurChain = frameDownsampleChain;
				//pipe->ps.frameBlurMipCount.x = frameDownsampleChain.texture().mipLevels();
			}
			else
			{
				//pipe->ps.frameBlurChain = TextureSRV();
				//pipe->ps.frameBlurMipCount.x = 0;
			}
			pipe->ps.cameraInverseProjectionMatrix = fromMatrix(cameraProjectionMatrix.inverse());
			pipe->ps.cameraInverseViewMatrix = fromMatrix(cameraViewMatrix.inverse());

			pipe->ps.ssao = m_blurTargetSRV;
            if (m_ssrEnabled)
                pipe->ps.ssr = m_ssrRenderer.ssr();
            else
                pipe->ps.ssr = TextureSRV();

			pipe->ps.frameSize.x = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).width());
			pipe->ps.frameSize.y = static_cast<uint32_t>(m_gbuffer->srv(GBufferType::Normal).height());

			pipe->ps.gbufferNormals = m_gbuffer->srv(GBufferType::Normal);
			pipe->ps.gbufferUV = m_gbuffer->srv(GBufferType::Uv);
			pipe->ps.gbufferInstanceId = m_gbuffer->srv(GBufferType::InstanceId);

			pipe->ps.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();
			//pipe->ps.objectIdToMaterialId = m_device.modelResources().gpuBuffers().objectIdToMaterial();
			pipe->ps.materialTextures = m_device.modelResources().textures();

			pipe->ps.probePositionRange = Float4{ probePosition.x, probePosition.y, probePosition.z, probeRange };
			pipe->ps.probeBBmin = Float4{ probePosition.x - probeRange, probePosition.y - probeRange, probePosition.z - probeRange, 0.0f };
			pipe->ps.probeBBmax = Float4{ probePosition.x + probeRange, probePosition.y + probeRange, probePosition.z + probeRange, 0.0f };

			pipe->ps.usingProbe.x = 0;
			pipe->ps.binSize.x = binSize(camera.width());
			pipe->ps.binSize.y = binSize(camera.height());

			if (lights.count() > 0)
			{
				pipe->ps.lightWorldPosition = lights.worldPositions();
				pipe->ps.lightDirection = lights.directions();
				pipe->ps.lightColor = lights.colors();
				pipe->ps.lightIntensity = lights.intensities();
				pipe->ps.lightRange = lights.ranges();
				pipe->ps.lightType = lights.types();
				pipe->ps.lightParameters = lights.parameters();
				pipe->ps.lightBins = m_shapeRenderer.lightBins();
				pipe->ps.lightIndexToShadowIndex = lightIndexToShadowIndex;
			}
			pipe->ps.exposure = camera.exposure();

            if (!pipe->ps.voxels)
                pipe->ps.voxels = m_device.createBindlessTextureSRV();

            if (m_sceneVoxelizer)
            {
                auto& grid = m_sceneVoxelizer->grids()[0];
                if (pipe->ps.voxels.size() == 0)
                {
                    pipe->ps.voxels.push(grid.voxelsOwner()[0]);
                    pipe->ps.voxels.push(grid.voxelsOwner()[1]);
                    pipe->ps.voxels.push(grid.voxelsOwner()[2]);
                    pipe->ps.voxels.push(grid.voxelsOwner()[3]);
                    pipe->ps.voxels.push(grid.voxelsOwner()[4]);
                }

                pipe->ps.voxelNormals = grid.normals();
                pipe->ps.voxelColorgrid = grid.colors();
                pipe->ps.voxelDepth = static_cast<float>(grid.voxels()[0].width());
                pipe->ps.voxelGridPosition = grid.position();
                pipe->ps.voxelGridSize = grid.size();
                pipe->ps.voxelMip.x = 0;
            }

			cmd.bindPipe(*pipe);
			cmd.draw(4u);
		}
    }

    void ModelRenderer::renderTemporalResolve(
        CommandList& cmd,
        TextureSRV depthView,
        Camera& camera,
        const Vector2f& jitterValue,
        const Vector2f& previousJitterValue,
        const Matrix4f& jitterMatrix)
    {
        CPU_MARKER(cmd.api(), "Temporal resolve");
        GPU_MARKER(cmd, "Temporal resolve");

        m_lastResolvedIndex = m_currentFullResIndex;
        TextureRTV target = m_fullResTargetFrame[m_currentFullResIndex];
        cmd.setRenderTargets({ target });

		engine::Pipeline<shaders::TemporalResolve>* pipe = nullptr;
		if (m_currentRenderMode == DEBUG_MODE_MOTION)
			pipe = &m_temporalResolve[1];
		else
			pipe = &m_temporalResolve[0];

        pipe->ps.currentFrame = m_lightingTargetSRV;
        pipe->ps.history = m_fullResTargetFrameSRV[previousFrameIndex()];
        pipe->ps.depth = depthView;
        pipe->ps.gbufferMotion = m_gbuffer->srv(GBufferType::Motion);
        pipe->ps.textureSize = Float2(static_cast<float>(target.width()), static_cast<float>(target.height()));
        pipe->ps.texelSize = Float2(1.0f / static_cast<float>(m_virtualResolution.x), 1.0f / static_cast<float>(m_virtualResolution.y));
        pipe->ps.nearFar = Float2(camera.nearPlane(), camera.farPlane());
        pipe->ps.jitter = jitterValue;
        pipe->ps.previousJitter = previousJitterValue;
        pipe->ps.inverseJitterMatrix = fromMatrix(jitterMatrix.inverse());

        cmd.bindPipe(*pipe);
        cmd.draw(4u);

        
    }

    void ModelRenderer::renderSSAO(
        Device& device,
        CommandList& cmd,
        TextureSRV depthView,
        Camera& camera)
    {
        CPU_MARKER(cmd.api(), "Render SSAO");
        GPU_MARKER(cmd, "Render SSAO");

        //cmd.clearRenderTargetView(m_ssaoRTV, Color4f{0.0f, 0.0f, 0.0f, 0.0f}); 
        cmd.setRenderTargets({ m_ssaoRTV });
#ifndef SCALEAOSIZE
        cmd.setViewPorts({ Viewport{ 0.0f, 0.0f, static_cast<float>(SSAOWidth), static_cast<float>(SSAOHeight), 0.0f, 1.0f } });
        cmd.setScissorRects({ Rectangle{ 0, 0, static_cast<unsigned int>(SSAOWidth), static_cast<unsigned int>(SSAOHeight) } });
#endif

        auto viewCornerRays = camera.viewRays();
        m_ssaoPipeline.vs.viewMatrix = fromMatrix(camera.viewMatrix());
        m_ssaoPipeline.vs.topLeftViewRay = Float4{ viewCornerRays.topLeft, 0.0f };
        m_ssaoPipeline.vs.topRightViewRay = Float4{ viewCornerRays.topRight, 0.0f };
        m_ssaoPipeline.vs.bottomLeftViewRay = Float4{ viewCornerRays.bottomLeft, 0.0f };
        m_ssaoPipeline.vs.bottomRightViewRay = Float4{ viewCornerRays.bottomRight, 0.0f };

        m_ssaoPipeline.ps.depthTexture = depthView;
        m_ssaoPipeline.ps.normalTexture = m_gbuffer->srv(GBufferType::Normal);
        m_ssaoPipeline.ps.noiseTexture = m_ssaoNoiseTexture;
        m_ssaoPipeline.ps.samples = m_ssaoKernelBuffer;

        m_ssaoPipeline.ps.cameraProjectionMatrix = fromMatrix(camera.projectionMatrix(Vector2<int>{ static_cast<int>(m_ssaoRTV.resource().width()), static_cast<int>(m_ssaoRTV.resource().height()) }));
        m_ssaoPipeline.ps.cameraViewMatrix = fromMatrix(camera.viewMatrix());
        m_ssaoPipeline.ps.nearFar = Float2{ camera.nearPlane(), camera.farPlane() };
#ifdef SCALEAOSIZE
        m_ssaoPipeline.ps.frameSize = Float2{ static_cast<float>(m_ssaoRTV.resource().width()), static_cast<float>(m_ssaoRTV.resource().height()) };
#else
        m_ssaoPipeline.ps.frameSize = Float2{ static_cast<float>(SSAOWidth), static_cast<float>(SSAOHeight) };
#endif

        cmd.bindPipe(m_ssaoPipeline);
        cmd.draw(4u);


#ifdef SCALEAOSIZE 
        if (!m_blurTarget || m_blurTarget.resource().width() != m_ssaoRTV.resource().width() || m_blurTarget.resource().height() != m_ssaoRTV.resource().height())
#else
        if (!m_blurTarget.valid() || m_blurTarget.width() != SSAOWidth || m_blurTarget.height() != SSAOHeight)
#endif
        {
            m_blurTarget = device.createTextureRTV(TextureDescription()
#ifdef SCALEAOSIZE
                .width(m_ssaoRTV.resource().width())
                .height(m_ssaoRTV.resource().height())
#else
                .width(SSAOWidth)
                .height(SSAOHeight)
#endif
                .format(engine::Format::R16_FLOAT)
                .usage(ResourceUsage::GpuRenderTargetReadWrite)
                .name("ssao blur target")
                .dimension(ResourceDimension::Texture2D)
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));

            m_blurTargetSRV = device.createTextureSRV(m_blurTarget);
        }

        {
            // blur the SSAO
            m_ssaoBlurPipeline.ps.image = m_ssaoSRV;
            m_ssaoBlurPipeline.ps.blurKernel = m_ssaoBlurKernelBuffer;
            m_ssaoBlurPipeline.ps.kernelSize.x = static_cast<int32_t>((m_ssaoBlurKernel.size() - 1) / 2);
#ifdef SCALEAOSIZE
            m_ssaoBlurPipeline.ps.texelSize = Float2(1.0f / static_cast<float>(m_ssaoRTV.resource().width()), 1.0f / static_cast<float>(m_ssaoRTV.resource().height()));
#else
            m_ssaoBlurPipeline.ps.texelSize = Float2(1.0f / static_cast<float>(SSAOWidth), 1.0f / static_cast<float>(SSAOHeight));
#endif

            cmd.setRenderTargets({ m_blurTarget });
#ifndef SCALEAOSIZE
            cmd.setViewPorts({ Viewport{ 0.0f, 0.0f, static_cast<float>(SSAOWidth), static_cast<float>(SSAOHeight), 0.0f, 1.0f } });
            cmd.setScissorRects({ Rectangle{ 0, 0, static_cast<unsigned int>(SSAOWidth), static_cast<unsigned int>(SSAOHeight) } });
#endif

            cmd.bindPipe(m_ssaoBlurPipeline);
            cmd.draw(4u);
        }

    }

	void ModelRenderer::renderSSAOForward(
		Device& device,
		CommandList& cmd,
		TextureSRV depthView,
		Camera& camera)
	{
		CPU_MARKER(cmd.api(), "Render SSAO");
		GPU_MARKER(cmd, "Render SSAO");

		cmd.setRenderTargets({ m_ssaoRTV });
#ifndef SCALEAOSIZE
		cmd.setViewPorts({ Viewport{ 0.0f, 0.0f, static_cast<float>(SSAOWidth), static_cast<float>(SSAOHeight), 0.0f, 1.0f } });
		cmd.setScissorRects({ Rectangle{ 0, 0, static_cast<unsigned int>(SSAOWidth), static_cast<unsigned int>(SSAOHeight) } });
#endif

		auto viewCornerRays = camera.viewRays();
		m_ssaoForwardPipeline.vs.viewMatrix = fromMatrix(camera.viewMatrix());
		m_ssaoForwardPipeline.vs.topLeftViewRay = Float4{ viewCornerRays.topLeft, 0.0f };
		m_ssaoForwardPipeline.vs.topRightViewRay = Float4{ viewCornerRays.topRight, 0.0f };
		m_ssaoForwardPipeline.vs.bottomLeftViewRay = Float4{ viewCornerRays.bottomLeft, 0.0f };
		m_ssaoForwardPipeline.vs.bottomRightViewRay = Float4{ viewCornerRays.bottomRight, 0.0f };

		m_ssaoForwardPipeline.ps.depthTexture = depthView;
		m_ssaoForwardPipeline.ps.noiseTexture = m_ssaoNoiseTexture;
		m_ssaoForwardPipeline.ps.samples = m_ssaoKernelBuffer;

		m_ssaoForwardPipeline.ps.cameraProjectionMatrix = fromMatrix(camera.projectionMatrix(Vector2<int>{ static_cast<int>(m_ssaoRTV.resource().width()), static_cast<int>(m_ssaoRTV.resource().height()) }));
		m_ssaoForwardPipeline.ps.cameraViewMatrix = fromMatrix(camera.viewMatrix());

		m_ssaoForwardPipeline.ps.invView = fromMatrix(camera.viewMatrix().inverse());
		m_ssaoForwardPipeline.ps.invProjection = fromMatrix(camera.projectionMatrix(Vector2<int>{ static_cast<int>(m_ssaoRTV.resource().width()), static_cast<int>(m_ssaoRTV.resource().height()) }).inverse());
		m_ssaoForwardPipeline.ps.cameraWorldSpacePosition = camera.position();

		m_ssaoForwardPipeline.ps.nearFar = Float2{ camera.nearPlane(), camera.farPlane() };
#ifdef SCALEAOSIZE
		m_ssaoForwardPipeline.ps.frameSize = Float2{ static_cast<float>(m_ssaoRTV.resource().width()), static_cast<float>(m_ssaoRTV.resource().height()) };
		m_ssaoForwardPipeline.ps.onePerFrameSize = Float2{ 1.0f / static_cast<float>(m_ssaoRTV.resource().width()), 1.0f / static_cast<float>(m_ssaoRTV.resource().height()) };
#else
		m_ssaoPipeline.ps.frameSize = Float2{ static_cast<float>(SSAOWidth), static_cast<float>(SSAOHeight) };
		m_ssaoPipeline.ps.onePerFrameSize = Float2{ 1.0f / static_cast<float>(SSAOWidth), 1.0f / static_cast<float>(SSAOHeight) };
#endif

		cmd.bindPipe(m_ssaoForwardPipeline);
		cmd.draw(4u);

#ifdef SCALEAOSIZE 
		if (!m_blurTarget || m_blurTarget.resource().width() != m_ssaoRTV.resource().width() || m_blurTarget.resource().height() != m_ssaoRTV.resource().height())
#else
		if (!m_blurTarget.valid() || m_blurTarget.width() != SSAOWidth || m_blurTarget.height() != SSAOHeight)
#endif
		{
			m_blurTarget = device.createTextureRTV(TextureDescription()
#ifdef SCALEAOSIZE
				.width(m_ssaoRTV.resource().width())
				.height(m_ssaoRTV.resource().height())
#else
				.width(SSAOWidth)
				.height(SSAOHeight)
#endif
				.format(engine::Format::R16_FLOAT)
				.usage(ResourceUsage::GpuRenderTargetReadWrite)
				.name("ssao blur target")
				.dimension(ResourceDimension::Texture2D)
				.optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }));

			m_blurTargetSRV = device.createTextureSRV(m_blurTarget);
		}

		{
			// blur the SSAO
			m_ssaoBlurPipeline.ps.image = m_ssaoSRV;
			m_ssaoBlurPipeline.ps.blurKernel = m_ssaoBlurKernelBuffer;
			m_ssaoBlurPipeline.ps.kernelSize.x = static_cast<int32_t>((m_ssaoBlurKernel.size() - 1) / 2);
#ifdef SCALEAOSIZE
			m_ssaoBlurPipeline.ps.texelSize = Float2(1.0f / static_cast<float>(m_ssaoRTV.resource().width()), 1.0f / static_cast<float>(m_ssaoRTV.resource().height()));
#else
			m_ssaoBlurPipeline.ps.texelSize = Float2(1.0f / static_cast<float>(SSAOWidth), 1.0f / static_cast<float>(SSAOHeight));
#endif

			cmd.setRenderTargets({ m_blurTarget });
#ifndef SCALEAOSIZE
			cmd.setViewPorts({ Viewport{ 0.0f, 0.0f, static_cast<float>(SSAOWidth), static_cast<float>(SSAOHeight), 0.0f, 1.0f } });
			cmd.setScissorRects({ Rectangle{ 0, 0, static_cast<unsigned int>(SSAOWidth), static_cast<unsigned int>(SSAOHeight) } });
#endif

			cmd.bindPipe(m_ssaoBlurPipeline);
			cmd.draw(4u);
		}
	}

	//static int SSRDebugPhase = 0;
	void ModelRenderer::renderSSRDebug(
		CommandList& cmd,
		TextureRTV currentRenderTarget,
		DepthPyramid& depthPyramid,
		int phase)
	{
		CPU_MARKER(cmd.api(), "Render SSR debug");
		GPU_MARKER(cmd, "Render SSR debug");

		cmd.setRenderTargets({ currentRenderTarget });

		m_ssrDebug.ps.depthPyramid = depthPyramid.depthMax();
		m_ssrDebug.ps.frame = m_fullResTransparencyTargetFrameSRV[m_currentFullResIndex];
		m_ssrDebug.ps.ssrDebugBuffer = m_ssrRenderer.ssrDebug();
		m_ssrDebug.ps.ssrDebugBufferCounter = m_ssrRenderer.ssrDebugCount();
		m_ssrDebug.ps.ssrDebugCount.x = phase;// (SSRDebugPhase / 40) % 20;
		m_ssrDebug.ps.frameSize = Float2{ static_cast<float>(currentRenderTarget.width()), static_cast<float>(currentRenderTarget.height()) };
        m_ssrDebug.ps.pow2size = Float2{
                static_cast<float>(roundUpToPow2(static_cast<int>(currentRenderTarget.width()))),
                static_cast<float>(roundUpToPow2(static_cast<int>(currentRenderTarget.height()))) };

		//++SSRDebugPhase;
		

		cmd.bindPipe(m_ssrDebug);
		cmd.draw(4u);
	}

#ifdef PARTICLE_TEST_ENABLED
    void ModelRenderer::renderParticles(
        Device& /*device*/,
        CommandList& cmd,
        TextureRTV rtv,
        TextureSRV dsvSRV,
        Camera& camera,
        LightData& lights,
        TextureSRV shadowMap,
        BufferSRV shadowVP)
    {
        m_particleTest.render(
            m_device, 
            cmd, 
            rtv, 
            dsvSRV, 
            camera, 
            m_projectionMatrix,
            m_viewMatrix,
            m_jitterMatrix,
            lights, 
            shadowMap,
            shadowVP);
    }
#endif

    void ModelRenderer::createSSAOSampleKernel()
    {
        std::uniform_real_distribution<float> randomFloat(0.0f, 1.0f);
        std::default_random_engine gen;

        auto lerp = [](float a, float b, float f)
        {
            return a + f * (b - a);
        };

        for (int i = 0; i < 64; ++i)
        {
            Vector3f sample{ 
                randomFloat(gen) * 2.0f - 1.0f,
                randomFloat(gen) * 2.0f - 1.0f,
                randomFloat(gen)
            };

            sample.normalize();
            sample *= randomFloat(gen);
            
            float scale = static_cast<float>(i) / 64.0f;
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            
            m_ssaoKernel.emplace_back(Vector4f(sample, 0.0f));
        }

        for (int i = 0; i < 16; ++i)
        {
            Vector3f temp{ 
                randomFloat(gen) * 2.0f - 1.0f,
                randomFloat(gen) * 2.0f - 1.0f,
                0.0f };
            temp.normalize();

            m_ssaoNoise.emplace_back(Vector4f{
                temp.x,
                temp.y,
                temp.z,
                0.0f
            });
        }
    }

    float normpdf(float x, float sigma)
    {
        return 0.39894f * exp(-0.5f * x * x / (sigma * sigma)) / sigma;
    }

    void ModelRenderer::createSSAOBlurKernel()
    {
        m_ssaoBlurKernel.resize(BilateralBlurSize);
        const int kSize = (BilateralBlurSize - 1) / 2;
        const float sigma = 10.0;
        for (int i = 0; i <= kSize; ++i)
        {
            m_ssaoBlurKernel[kSize + i] = m_ssaoBlurKernel[kSize - i] = normpdf(static_cast<float>(i), sigma);
        }
    }

    void ModelRenderer::renderVoxels(
        CommandList& cmd, 
        FlatScene& scene, 
        TextureSRV shadowMap,
        BufferSRV shadowVP,
        BufferSRV lightIndexToShadowIndex,
        LightData& lights)
    {
        if (m_voxelize && !m_sceneVoxelizer)
        {
            m_sceneVoxelizer = engine::make_unique<SceneVoxelizer>(m_device);
        }
        else if (!m_voxelize && m_sceneVoxelizer)
        {
            m_sceneVoxelizer = nullptr;
        }

        if (m_sceneVoxelizer)
        {
            m_sceneVoxelizer->voxelize(cmd, scene.drawCamera());
            m_sceneVoxelizer->lightVoxels(cmd, shadowMap, shadowVP, lightIndexToShadowIndex, lights);

            if (m_debugVoxels)
            {
                m_sceneVoxelizer->createDebugVoxelData(
                    cmd,
                    scene.drawCamera(),
                    m_debugVoxelMip);
            }
            else
                m_sceneVoxelizer->clearDebug();
        }
    }

    void ModelRenderer::renderTerrain(
        CommandList& cmd,
        FlatScene& scene,
        TextureRTV currentRenderTarget,
        TextureDSV currentDepthTarget,
        TextureSRV shadowMap,
        BufferSRV shadowVP,
        BufferSRV lightIndexToShadowIndex,
        LightData& lights)
    {
        Vector3f probePosition;
        float probeRange = 100.0f;
        if (scene.probes.size() > 0)
        {
            ProbeComponent& probe = *scene.probes[0];
            probePosition = probe.position();
            probeRange = probe.range();
        }

		for (auto&& terrain : scene.terrains)
		{
			terrain->terrain().render(
				cmd,
				scene.drawCamera(),
				currentRenderTarget,
				currentDepthTarget,
				shadowMap,
				shadowVP,
				lightIndexToShadowIndex,
				m_blurTargetSRV,
				probePosition,
				probeRange,
				static_cast<uint32_t>(m_currentRenderMode),
				m_virtualResolution,
				lights,
				m_shapeRenderer.lightBins(),
				m_terrainDebugMode == 0 || m_terrainDebugMode == 2,
				m_terrainDebugMode == 1 || m_terrainDebugMode == 2,
				terrain->transform().position());
		}
    }

    void ModelRenderer::renderDebugVoxels(
        CommandList& cmd,
        FlatScene& scene,
        TextureRTV currentRenderTarget,
        TextureDSV currentDepthTarget)
    {
        if (m_sceneVoxelizer)
        {
            if (m_debugVoxels)
            {
                m_sceneVoxelizer->renderDebug(
                    cmd,
                    scene.drawCamera(),
                    currentRenderTarget,
                    currentDepthTarget,
                    m_debugVoxelGrids);
            }
        }
    }
}
