#pragma once

#include "shaders/core/renderers/forward/RenderClustersForward.h"
#include "shaders/core/renderers/forward/RenderClustersForwardJitterEqual.h"
#include "shaders/core/renderers/forward/RenderClustersForwardJitterAlphaClippedEqual.h"
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
	class TextureRTV;
	class LightData;

	class RenderForward
	{
	public:
		RenderForward(Device& device);

		void render(
			CommandList& cmd,
			ClusterDataLine& clusters,
			IndexDataLine& indexes,
			DrawDataLine& draws,
			TextureDSV depth,
			TextureRTV rtv,
			TextureRTV motion,
			TextureSRV shadowMap,
			BufferSRV shadowVP,
			BufferSRV lightIndexToShadowIndex,
			TextureSRV ssao,
			Camera& camera,
			Vector3f probePosition,
			float probeRange,
			JitterOption jitter,
			AlphaClipOption alphaClipped,
			DepthTestOption depthTest,
			uint32_t debugMode,
			const Vector2<int>& virtualResolution,
			LightData& lights,
			BufferSRV lightBins,
			const Matrix4f& previousCameraViewMatrix,
			const Matrix4f& previousCameraProjectionMatrix);

	private:
		Device& m_device;
		engine::Pipeline<shaders::RenderClustersForward> m_defaultGreaterEqual;
		engine::Pipeline<shaders::RenderClustersForward> m_defaultGreaterEqualDebug;

		engine::Pipeline<shaders::RenderClustersForward> m_jitterGreaterEqual;
		engine::Pipeline<shaders::RenderClustersForward> m_jitterGreaterEqualDebug;

		engine::Pipeline<shaders::RenderClustersForward> m_alphaClippedGreaterEqual;
		engine::Pipeline<shaders::RenderClustersForward> m_alphaClippedGreaterEqualDebug;

		engine::Pipeline<shaders::RenderClustersForward> m_jitterAlphaClippedGreaterEqual;
		engine::Pipeline<shaders::RenderClustersForward> m_jitterAlphaClippedGreaterEqualDebug;

		engine::Pipeline<shaders::RenderClustersForward> m_defaultEqual;
		engine::Pipeline<shaders::RenderClustersForward> m_defaultEqualDebug;

		engine::Pipeline<shaders::RenderClustersForwardJitterEqual> m_jitterEqual;
		engine::Pipeline<shaders::RenderClustersForwardJitterEqual> m_jitterEqualDebug;

		engine::Pipeline<shaders::RenderClustersForward> m_alphaClippedEqual;
		engine::Pipeline<shaders::RenderClustersForward> m_alphaClippedEqualDebug;

		engine::Pipeline<shaders::RenderClustersForwardJitterAlphaClippedEqual> m_jitterAlphaClippedEqual;
		engine::Pipeline<shaders::RenderClustersForwardJitterAlphaClippedEqual> m_jitterAlphaClippedEqualDebug;
	};
}
