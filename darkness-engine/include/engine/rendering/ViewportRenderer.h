#pragma once

#include "engine/rendering/ModelRenderer.h"
#include "engine/rendering/ShadowRenderer.h"
#include "engine/rendering/RenderCubemap.h"
#include "engine/rendering/Postprocess.h"
#include "engine/rendering/DepthPyramid.h"
#include "engine/rendering/debug/DebugBoundingBoxes.h"

#include "engine/rendering/LightData.h"

namespace engine
{
    class Device;
    class Camera;
    class CommandList;
    struct FlatScene;

    class ViewportRenderer
    {
    public:
        ViewportRenderer(
            Device& device, 
            int width,
            int height);

        void render(
            CommandList& cmd, 
            FlatScene& scene, 
            TextureRTV destination,
            unsigned int mouseX,
            unsigned int mouseY);

        unsigned int pickedObject(Device& device);
        void setSelectedObject(int64_t object);

		void setSSRDebugMousePosition(int x, int y);

        TextureDSV dsv()
        {
            return m_depthPyramid.dsv();
        }

        RenderCubemap& cubemapRenderer()
        {
            return m_renderCubemap;
        }

        ShadowRenderer& shadowRenderer()
        {
            return m_shadowRenderer;
        }

        void refresh(engine::Vector2<int> virtualResolution);

        bool* measuresEnabled()
        {
            return m_modelRenderer.measuresEnabled();
        }

        bool* bufferStatsEnabled()
        {
            return m_modelRenderer.gpuBufferStatsEnabled();
        }

        bool* cullingStatsEnabled()
        {
            return m_modelRenderer.gpuCullingStatsEnabled();
        }

        bool* logEnabled()
        {
            return m_modelRenderer.logEnabled();
        }

		TextureSRV ssrResult();
        FrameStatistics getStatistics();

		Vector2<int> virtualResolution() const;
		bool virtualResolutionChange(bool reset);

		int ssrDebugPhase() const
		{
			return m_ssrDebugPhase;
		}
		void ssrDebugPhase(int ssrDebugPhase)
		{
			m_ssrDebugPhase = ssrDebugPhase;
		}
    private:
        Device& m_device;

        TextureRTVOwner m_rtv;
        TextureSRVOwner m_srv;

        DepthPyramid m_depthPyramid;
        ModelRenderer m_modelRenderer;
        ShadowRenderer m_shadowRenderer;
        RenderCubemap m_renderCubemap;
        Postprocess m_postProcess;
		DebugBoundingBoxes m_debugBoundingBoxes;

		int m_ssrDebugPhase;
    };
}
