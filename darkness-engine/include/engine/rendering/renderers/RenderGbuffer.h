#pragma once

#include "shaders/core/renderers/gbuffer/RenderClustersGbuffer.h"
#include "shaders/core/renderers/gbuffer/RenderClustersGbufferAlphaClipped.h"

#include "engine/rendering/renderers/RenderersCommon.h"
#include "engine/graphics/Pipeline.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/primitives/Vector2.h"

namespace engine
{
	class Device;
	class CommandList;
	class Camera;
	class TextureDSV;
	class GBuffer;

	class RenderGbuffer
	{
	public:
		RenderGbuffer(Device& device);

		void render(
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
			const Matrix4f& previousCameraProjectionMatrix);

	private:
		Device & m_device;

		engine::Pipeline<shaders::RenderClustersGbuffer>					m_default;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_alphaClipped;
		engine::Pipeline<shaders::RenderClustersGbuffer>					m_jitter;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_jitterAlphaClipped;
		engine::Pipeline<shaders::RenderClustersGbuffer>					m_defaultBias;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_alphaClippedBias;
		engine::Pipeline<shaders::RenderClustersGbuffer>					m_jitterBias;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_jitterAlphaClippedBias;

		engine::Pipeline<shaders::RenderClustersGbuffer>					m_equaldefault;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_equalalphaClipped;
		engine::Pipeline<shaders::RenderClustersGbuffer>					m_equaljitter;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_equaljitterAlphaClipped;
		engine::Pipeline<shaders::RenderClustersGbuffer>					m_equaldefaultBias;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_equalalphaClippedBias;
		engine::Pipeline<shaders::RenderClustersGbuffer>					m_equaljitterBias;
		engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_equaljitterAlphaClippedBias;

        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugDefault;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugAlphaClipped;
        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugJitter;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugJitterAlphaClipped;
        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugDefaultBias;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugAlphaClippedBias;
        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugJitterBias;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugJitterAlphaClippedBias;
                                                                              
        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugEqualdefault;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugEqualalphaClipped;
        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugEqualjitter;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugEqualjitterAlphaClipped;
        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugEqualdefaultBias;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugEqualalphaClippedBias;
        engine::Pipeline<shaders::RenderClustersGbuffer>					m_debugEqualjitterBias;
        engine::Pipeline<shaders::RenderClustersGbufferAlphaClipped>		m_debugEqualjitterAlphaClippedBias;
	};
}
