#pragma once

/*#include "shaders/core/dxr/ClosestHit.h"
#include "shaders/core/dxr/Miss.h"
#include "shaders/core/dxr/Raygeneration.h"*/

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

	class RenderDxr
	{
	public:
		RenderDxr(Device& device);

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
		/*engine::Pipeline<shaders::ClosestHit> m_closestHit;
		engine::Pipeline<shaders::Miss> m_miss;
		engine::Pipeline<shaders::Raygeneration> m_raygeneration;*/

		uint32_t m_lastIndexBufferSize;

		RaytracingAccelerationStructureOwner m_rayAccelerationStructure;
	};
}
