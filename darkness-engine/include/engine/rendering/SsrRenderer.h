#pragma once

#include "engine/graphics/Device.h"
#include "shaders/core/ssr/SSR.h"
#include "shaders/core/ssr/SSRForward.h"
#include "shaders/core/ssr/GBufferRoughness.h"
//#include "shaders/core/tools/Blur.h"

#include <random>

namespace engine
{
    class CommandList;
    class TextureSRV;
	class TextureRTV;
    class Camera;
	class GBuffer;
	class DepthPyramid;

    class SsrRenderer
    {
    public:
        SsrRenderer(Device& device);

        void render(
            Device& device,
            CommandList& cmd,
			DepthPyramid& depthPyramid,
			TextureSRV frame,
			GBuffer& gbuffer,
            Camera& camera);

		void renderForward(
			Device& device,
			CommandList& cmd,
			TextureSRV depthView,
			TextureSRV frame,
			Camera& camera);

		TextureSRV ssr();

		BufferSRV ssrDebug();
		BufferSRV ssrDebugCount();
		void setSSRDebugMousePosition(int x, int y);
    private:
        Device & m_device;
        engine::Pipeline<shaders::SSR> m_pipeline;
		engine::Pipeline<shaders::SSRForward> m_pipelineForward;
		engine::Pipeline<shaders::GBufferRoughness> m_roughnessPipeline;
		//engine::Pipeline<shaders::Blur> m_blur;

		void createBuffers();
		void resizeTarget(CommandList& cmd, size_t width, size_t height);
		TextureRTVOwner m_rtv;
		TextureSRVOwner m_srv;
		/*TextureUAVOwner m_uav;
		TextureUAVOwner m_blurUav;
		TextureSRVOwner m_blurSrv;*/

		TextureRTVOwner m_roughnessRTV;
		TextureSRVOwner m_roughnessSRV;

		TextureSRVOwner m_noiseTexture;

		BufferUAVOwner m_ssrDebugUAV;
		BufferSRVOwner m_ssrDebugSRV;
		BufferUAVOwner m_ssrDebugCounterUAV;
		BufferSRVOwner m_ssrDebugCounterSRV;

		std::default_random_engine m_generator;
		std::uniform_real_distribution<float> m_distribution;

		void test(float2 frameSize);

		int m_mousexSSRDebug;
		int m_mouseySSRDebug;
    };
}