#pragma once

#include "engine/rendering/BufferSettings.h"
#include "engine/graphics/CommandList.h"
#include "components/Transform.h"
#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/LightComponent.h"
#include "components/Camera.h"
#include "shaders/core/lighting/Lighting.h"
#include "shaders/core/ssao/SSAO.h"
#include "shaders/core/ssao/SSAOForward.h"
#include "shaders/core/ssao/SSAOBlur.h"
#include "shaders/core/temporal/TemporalResolve.h"
#include "shaders/core/tools/Wireframe.h"
#include "shaders/core/tools/DebugBoundingSpheres.h"
#include "engine/rendering/RenderOutline.h"
#include "engine/Scene.h"
#include "engine/rendering/LightData.h"
#include "engine/rendering/GBuffer.h"
#include "engine/rendering/ModelResources.h"
#include "engine/rendering/ParticleTest.h"
#include "engine/rendering/Picker.h"
#include "engine/rendering/ClusterRendererTransparent.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/culling/FrustumCuller.h"
#include "engine/rendering/culling/OcclusionCuller.h"
#include "engine/rendering/culling/IndexExpansion.h"
#include "engine/rendering/tools/BufferMath.h"
#include "engine/rendering/renderers/RenderDepth.h"
#include "engine/rendering/renderers/RenderForward.h"
#include "engine/rendering/renderers/RenderGbuffer.h"
#include "engine/rendering/renderers/RenderDxr.h"
#include "engine/rendering/SsrRenderer.h"
#include "engine/rendering/SceneVoxelizer.h"

#include "engine/rendering/EcsDemo.h"
#include "engine/rendering/TerrainRenderer.h"

#include "shaders/core/ssr/SSRDebug.h"
#include "shaders/core/shared_types/Statistics.hlsli"

#include "containers/memory.h"

#include "engine/rendering/ShapeRenderer.h"
#include "engine/rendering/ShapeMeshFactory.h"

#define SCALEAOSIZE
#undef PARTICLE_TEST_ENABLED
#define ECS_TEST_ENABLED

namespace engine
{
#ifdef SCALEAOSIZE
    constexpr int SSAOresolutionScaleFactor = 2;
#else
    constexpr int SSAOWidth = 1600;
    constexpr int SSAOHeight = 1200;
#endif
    constexpr int HistoryCount = 2;
    constexpr int BilateralBlurSize = 15;

    constexpr int DataLineCount = 2;

    class ModelRenderer
    {
    public:
        ModelRenderer(Device& device, Vector2<int> virtualResolution);
        void render(
            Device& device,
            DepthPyramid& depthPyramid,
            CommandList& cmd, 
            FlatScene& scene,
            // these are for voxel lighting
            TextureSRV shadowMap,
            BufferSRV shadowVP,
            BufferSRV lightIndexToShadowIndex,
            LightData& lights);

		void renderForward(
			Device& device,
			DepthPyramid& depthPyramid,
			CommandList& cmd,
			FlatScene& scene,
			TextureSRV shadowMap,
			BufferSRV shadowVP,
			BufferSRV lightIndexToShadowIndex,
			LightData& lights);

		void renderLightBins(
			Device& device,
			CommandList& cmd,
			DepthPyramid& depthPyramid,
            FlatScene& scene,
			LightData& lights);

        void renderPicker(
            CommandList& cmd,
            unsigned int mouseX,
            unsigned int mouseY);
        void renderSSAO(
            Device& device, 
            CommandList& cmd, 
            DepthPyramid& depthPyramid,
            FlatScene& scene);
		void renderSSAOForward(
			Device& device,
			CommandList& cmd,
			DepthPyramid& depthPyramid,
            FlatScene& scene);

		void renderFrameDownsample(
			CommandList& cmd);

		TextureSRV ssrResult();
		void renderSSR(
			Device& device,
			CommandList& cmd,
			DepthPyramid& depthPyramid,
            FlatScene& scene);
		void renderSSRForward(
			Device& device,
			CommandList& cmd,
			DepthPyramid& depthPyramid,
			Camera& camera);

        void renderTransparent(
            Device& device,
            DepthPyramid& depthPyramid,
            CommandList& cmd,
            FlatScene& scene,
            TextureSRV shadowMap,
            BufferSRV shadowVP);
        void renderLighting(
            CommandList& cmd, 
            FlatScene& scene,
            DepthPyramid& depthPyramid,
            TextureSRV shadowMap,
            BufferSRV shadowVP,
			BufferSRV lightIndexToShadowIndex);
        void renderOutline(
            Device& device,
            CommandList& cmd,
            FlatScene& scene,
            DepthPyramid& depthPyramid,
            Camera& camera);
        void renderTemporalResolve(
            CommandList& cmd,
            DepthPyramid& depthPyramid,
            Camera& camera);
		void renderSSRDebug(
			CommandList& cmd,
			TextureRTV currentRenderTarget,
			DepthPyramid& depthPyramid,
			int phase);
        void renderTerrain(
            CommandList& cmd,
            FlatScene& scene,
            TextureRTV currentRenderTarget,
            TextureDSV currentDepthTarget,
            TextureSRV shadowMap,
            BufferSRV shadowVP,
            BufferSRV lightIndexToShadowIndex,
            LightData& lights);
        void renderDebugVoxels(
            CommandList& cmd,
            FlatScene& scene,
            TextureRTV currentRenderTarget,
            TextureDSV currentDepthTarget);

#ifdef PARTICLE_TEST_ENABLED
        void renderParticles(
            Device& /*device*/,
            CommandList& cmd,
            TextureRTV rtv,
            TextureSRV dsvSRV,
            Camera& camera,
            LightData& lights,
            TextureSRV shadowMap,
            BufferSRV shadowVP
        );
#endif

        void flip();

        void resize(uint32_t width, uint32_t height);

        TextureSRV ssaoSRV()
        {
            return m_ssaoSRV;
        }

        TextureSRV finalFrame()
        {
            return m_fullResTargetFrameSRV[m_lastResolvedIndex];
        }

        TextureRTV finalFrameRTV()
        {
            return m_fullResTargetFrame[m_lastResolvedIndex];
        }

        TextureRTV lightingTargetRTV()
        {
            return getCurrentLightingTarget();
        }

        TextureDSV debugDepth()
        {
            return m_debugDepth;
        }

        unsigned int pickedObject(Device& device);
        void setSelectedObject(int64_t object)
        {
            m_selectedObject = object;
        }

		void setSSRDebugMousePosition(int x, int y);

        bool* measuresEnabled()
        {
            return &m_gpuMeasuresEnabled;
        }

        bool* gpuBufferStatsEnabled()
        {
            return &m_gpuBufferStatsEnabled;
        }

        bool* gpuCullingStatsEnabled()
        {
            return &m_gpuCullingStatsEnabled;
        }

        bool* logEnabled()
        {
            return &m_logEnabled;
        }

		bool* forwardRendering()
		{
			return &m_forwardRendering;
		}

        bool histogramDebug() const
        {
            return m_histogramDebug;
        }

		bool debugBoundingBoxes() const
		{
			return m_debugBoundingBoxes;
		}

        bool taaEnabled() const { return m_taaEnabled; }
        void taaEnabled(bool enabled) { m_taaEnabled = enabled; }

        FrameStatistics getStatistics();

		Vector2<int> virtualResolution() const;
		bool virtualResolutionChange(bool reset);

    private:
        engine::Matrix4f m_viewMatrix;
        engine::Matrix4f m_projectionMatrix;
        engine::Matrix4f m_jitterMatrix;
        engine::Vector2f m_jitterValue;
    private:

        void renderGeometry(
            Device& device,
            TextureRTV currentRenderTarget,
            TextureDSV depthBuffer,
            CommandList& cmd,
            FlatScene& scene,
            Matrix4f cameraProjectionMatrix,
            Matrix4f cameraViewMatrix,
            Camera& camera,
            LightData& lights,
            TextureSRV shadowMap,
            BufferSRV shadowVP);

        void renderLighting(
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
            TextureSRV frameDownsampleChain);

        void renderSSAO(
            Device& device,
            CommandList& cmd,
            TextureSRV depthView,
            Camera& camera);

		void renderSSAOForward(
			Device& device,
			CommandList& cmd,
			TextureSRV depthView,
			Camera& camera);

        void createSSAOSampleKernel();
        void createSSAOBlurKernel();

        void renderTemporalResolve(
            CommandList& cmd,
            TextureSRV depthView,
            Camera& camera,
            const Vector2f& jitterValue,
            const Vector2f& previousJitterValue,
            const Matrix4f& jitterMatrix);

		void renderDebugMenu();

		struct DataLineSetup
		{
			uint32_t currentDataLine;
			uint32_t previousDataLine;
			uint32_t alphaClipDataLine;
			uint32_t alphaClipDataLineSecond;
			uint32_t transparentDataLine;
			uint32_t transparentDataLineSecond;
			uint32_t terrainDataLine;
			uint32_t terrainDataLineSecond;
		};
		void resetDataLines(CommandList& cmd, DataLineSetup dataline);
		void clearDepth(CommandList& cmd, DepthPyramid& depthPyramid);
		void frustumCullPreviousClusters(CommandList& cmd, DataLineSetup dataline, Camera& cullingCamera);
		void drawFirstPassDeferred(
            CommandList& cmd, 
            DataLineSetup dataline, 
            DepthPyramid& depthPyramid, 
            Camera& drawCamera, 
            FlatScene& scene,
            // these are for voxel lighting
            TextureSRV shadowMap,
            BufferSRV shadowVP,
            BufferSRV lightIndexToShadowIndex,
            LightData& lights);
        void drawFirstPassDeferredDebug(CommandList& cmd, DataLineSetup dataline, DepthPyramid& depthPyramid, Camera& drawCamera, Camera& cullCamera, FlatScene& scene);
		void drawFirstPassForward(
            CommandList& cmd, 
            DataLineSetup dataline, 
            DepthPyramid& depthPyramid, 
            FlatScene& scene,
			TextureSRV shadowMap,
			BufferSRV shadowVP,
			BufferSRV lightIndexToShadowIndex,
			LightData& lights,
			const Vector3f& probePosition, float probeRange);
        void drawFirstPassForwardDebug(
            CommandList& cmd, 
            DataLineSetup dataline, 
            DepthPyramid& depthPyramid, 
            FlatScene& scene,
            TextureSRV shadowMap,
            BufferSRV shadowVP,
            BufferSRV lightIndexToShadowIndex,
            LightData& lights,
            const Vector3f& probePosition, float probeRange);
		void createDepthPyramid(CommandList& cmd, DepthPyramid& depthPyramid);
		void copyFirstPassStatistics(CommandList& cmd, DataLineSetup dataline);
		void frustumInstanceCull(CommandList& cmd, DataLineSetup dataline, DepthPyramid& depthPyramid, Camera& cullingCamera);
		void expandClusters(CommandList& cmd);
		void occlusionCull(CommandList& cmd, DataLineSetup dataline, DepthPyramid& depthPyramid, Camera& cullingCamera);
		void drawSecondPassDeferred(
			CommandList& cmd, 
			DataLineSetup dataline, 
			DepthPyramid& depthPyramid, 
			Camera& drawCamera);
        void drawSecondPassDeferredDebug(
            CommandList& cmd,
            DataLineSetup dataline,
            Camera& drawCamera);
		void drawSecondPassForward(
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
			FlatScene& scene);
        void drawSecondPassForwardDebug(
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
			FlatScene& scene);
		void copySecondPassStatistics(CommandList& cmd, DataLineSetup dataline);
		void resetBloomFilter(CommandList& cmd);
        void renderVoxels(
            CommandList& cmd, 
            FlatScene& scene, 
            TextureSRV shadowMap,
            BufferSRV shadowVP,
            BufferSRV lightIndexToShadowIndex,
            LightData& lights);

    private:
        Device& m_device;
        BufferMath m_bufferMath;
        ClusterRendererTransparent m_transparent;
		ShapeRenderer m_shapeRenderer;
		RenderDepth m_renderDepth;
		RenderForward m_renderForward;
		RenderGbuffer m_renderGbuffer;
		RenderDxr m_renderDxr;
		SsrRenderer m_ssrRenderer;
        
#ifdef ECS_TEST_ENABLED
		EcsDemo m_ecsDemo;
#endif

        ClusterDataLine m_clusterDataLine[8];
        IndexDataLine   m_indexDataLine;
        DrawDataLine    m_drawDataLine[8];

        uint32_t        m_currentDataLine;

        struct Phase1
        {
            Phase1(Device& device)
                : frustumClusterCullDraw{ device }
                , frustumClusterCullDrawAlphaClipped{ device }
                , frustumClusterCullDrawTransparent{ device }
                , frustumClusterCullDrawTerrain{ device }
                , depthIndexDraw{ device }
                , alphaClipIndexDraw{ device }
                , transparentIndexDraw{ device }
                , terrainIndexDraw{ device }
            {}
            ClusterDataLine frustumClusterCullDraw;
            ClusterDataLine frustumClusterCullDrawAlphaClipped;
            ClusterDataLine frustumClusterCullDrawTransparent;
			ClusterDataLine frustumClusterCullDrawTerrain;
            IndexDataLine depthIndexDraw;
            IndexDataLine alphaClipIndexDraw;
            IndexDataLine transparentIndexDraw;
			IndexDataLine terrainIndexDraw;
        };
        Phase1 m_phase1;

        struct Phase2
        {
            Phase2(Device& device)
                : frustumInstanceCullDraw{ device }
                , occlusionCullDrawAll{ device }
                , occlusionCullDrawNotYetDrawnDepth{ device }
                , occlusionCullDrawAlphaClipped{ device }
                , occlusionCullDrawTransparent{ device }
                , occlusionCullDrawTerrain{ device }
                , notYetDrawnDepthIndexDraw{ device }
                , alphaClipIndexDraw{ device }
                , transparentIndexDraw{ device }
                , terrainIndexDraw{ device }
            {}
            ClusterDataLine frustumInstanceCullDraw;

            ClusterDataLine occlusionCullDrawAll;
            ClusterDataLine occlusionCullDrawNotYetDrawnDepth;
            ClusterDataLine occlusionCullDrawAlphaClipped;
            ClusterDataLine occlusionCullDrawTransparent;
			ClusterDataLine occlusionCullDrawTerrain;

            IndexDataLine notYetDrawnDepthIndexDraw;
            IndexDataLine alphaClipIndexDraw;
            IndexDataLine transparentIndexDraw;
			IndexDataLine terrainIndexDraw;
        };
        Phase2 m_phase2;

		DataLineSetup setupDataLines();

        Vector2<int> m_virtualResolution;
        
        FrustumCuller m_frustumCuller;
        OcclusionCuller m_occlusionCuller;
        IndexExpansion m_indexExpansion;

        engine::unique_ptr<SceneVoxelizer> m_sceneVoxelizer;

        Picker m_picker;
        FrameStatistics m_statistics;
        size_t m_modelInstanceCount;
        size_t m_activeClusterCount;
        size_t m_activeIndexCount;
#ifdef PARTICLE_TEST_ENABLED
        ParticleTest m_particleTest;
#endif

        engine::Pipeline<shaders::Lighting> m_lightingPipeline;
		engine::Pipeline<shaders::Lighting> m_debugLightingPipeline;
        engine::Pipeline<shaders::Lighting> m_lightingPipelineVoxelReflections;
        engine::Pipeline<shaders::Lighting> m_debugLightingPipelineVoxelReflections;

        engine::vector<engine::Pipeline<shaders::TemporalResolve>> m_temporalResolve;

        int m_currentRenderMode;

        int m_renderWidth;
        int m_renderHeight;
        unsigned int m_frameNumber;

        engine::unique_ptr<engine::RenderOutline> m_renderOutline;

        engine::shared_ptr<GBuffer> m_gbuffer;

        BufferSRV m_shadowsSRV;

        /*BufferUAV m_pickBufferUAV;
        BufferSRV m_pickBufferReadBack;*/

        engine::Pipeline<shaders::SSAO> m_ssaoPipeline;
		engine::Pipeline<shaders::SSAOForward> m_ssaoForwardPipeline;
        engine::vector<Vector4f> m_ssaoKernel;
        engine::vector<Vector4f> m_ssaoNoise;
        TextureSRVOwner m_ssaoNoiseTexture;
        BufferSRVOwner m_ssaoKernelBuffer;
        TextureRTVOwner m_ssaoRTV;
        TextureSRVOwner m_ssaoSRV;
        engine::vector<float> m_ssaoBlurKernel;
        BufferSRVOwner m_ssaoBlurKernelBuffer;

        engine::Pipeline<shaders::SSAOBlur> m_ssaoBlurPipeline;
        TextureRTVOwner m_blurTarget;
        TextureSRVOwner m_blurTargetSRV;

        TextureRTVOwner m_lightingTarget;
        TextureSRVOwner m_lightingTargetSRV;
        TextureUAVOwner m_lightingTargetUAV;
        TextureRTVOwner m_fullResTargetFrame[HistoryCount];
        TextureSRVOwner m_fullResTargetFrameSRV[HistoryCount];
		TextureUAVOwner m_fullResTargetFrameUAV[HistoryCount];
        TextureRTVOwner m_fullResTransparencyTargetFrame[HistoryCount];
        TextureSRVOwner m_fullResTransparencyTargetFrameSRV[HistoryCount];
		TextureUAVOwner m_fullResTransparencyTargetFrameUAV[HistoryCount];
        int m_currentFullResIndex;
        int m_lastResolvedIndex;

        TextureDSVOwner m_dsv;
        TextureSRVOwner m_dsvSRV;

        int previousFrameIndex()
        {
            auto prevIndex = m_currentFullResIndex - 1;
            if (prevIndex < 0)
                prevIndex = HistoryCount - 1;
            return prevIndex;
        }

        Matrix4f m_previousCameraProjectionMatrix;
        Matrix4f m_previousCameraViewMatrix;
        Matrix4f m_previousJitterMatrix;
        Vector2f m_previousJitter;

        int64_t m_selectedObject;

        bool m_taaEnabled;
        bool m_ssaoEnabled;
        bool m_ssrEnabled;
        bool m_vsyncEnabled;
        bool m_gpuMeasuresEnabled;
        bool m_gpuBufferStatsEnabled;
        bool m_gpuCullingStatsEnabled;
        bool m_logEnabled;
        bool m_debugMenuOpen;
		bool m_forwardRendering;
        bool m_voxelize;
        bool m_debugVoxels;
        bool m_debugVoxelGrids;
        bool m_renderTransparency;
        bool m_histogramDebug;
		bool m_debugBoundingBoxes;
		bool m_terrainAllDebugBoundingBoxes;
		int m_debugVoxelMip;
		int m_virtualResolutionImgui[2];
		bool m_virtualResolutionImguiChanged;
        int m_terrainDebugMode;

        void reallocateClusterDatalines();

		BufferSRVOwner m_cameraInLights;
		BufferUAVOwner m_cameraInLightsUAV;

        BufferOwner m_frameStatistics;

		Shape m_testShape;
		GpuShape m_testGpuShape;

		engine::Pipeline<shaders::SSRDebug> m_ssrDebug;

        TextureDSVOwner m_debugDepth;
        TextureSRVOwner m_debugDepthSRV;

        TextureRTV getCurrentLightingTarget();
        TextureSRV getCurrentLightingTargetSRV();
        TextureUAV getCurrentLightingTargetUAV();
    };
}
