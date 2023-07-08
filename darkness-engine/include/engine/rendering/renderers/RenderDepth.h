#pragma once

#include "shaders/core/renderers/depth/RenderDepthDefault.h"
#include "shaders/core/renderers/depth/RenderDepthAlphaClipped.h"
#include "shaders/core/renderers/depth/RenderDepthJitter.h"
#include "shaders/core/renderers/depth/RenderDepthJitterAlphaClipped.h"

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

	class RenderDepth
	{
	public:
		RenderDepth(Device& device);

		void render(
			CommandList& cmd,
			ClusterDataLine& clusters,
			IndexDataLine& indexes,
			DrawDataLine& draws,
			TextureDSV depth,
			Camera& camera,
			JitterOption jitter,
			AlphaClipOption alphaClipped,
			BiasOption bias,
			const Vector2<int>& virtualResolution);

	private:
		Device& m_device;
		engine::Pipeline<shaders::RenderDepthDefault>					m_default;
		engine::Pipeline<shaders::RenderDepthAlphaClipped>				m_alphaClipped;
		engine::Pipeline<shaders::RenderDepthJitter>					m_jitter;
		engine::Pipeline<shaders::RenderDepthJitterAlphaClipped>		m_jitterAlphaClipped;

		engine::Pipeline<shaders::RenderDepthDefault>					m_defaultBias;
		engine::Pipeline<shaders::RenderDepthAlphaClipped>				m_alphaClippedBias;
		engine::Pipeline<shaders::RenderDepthJitter>					m_jitterBias;
		engine::Pipeline<shaders::RenderDepthJitterAlphaClipped>		m_jitterAlphaClippedBias;

		TextureRTVOwner m_vittuihansama;
		void createIhanVitunSama(int width, int height);
	};
}
