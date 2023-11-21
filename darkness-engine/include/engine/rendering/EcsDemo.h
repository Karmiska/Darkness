#pragma once

#undef ECSTEST22

#include "engine/graphics/Pipeline.h"
#include "shaders/core/shape/RenderCubes.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/primitives/Matrix4.h"
#include "engine/rendering/ShapeMeshFactory.h"

#ifdef ECSTEST22
#include "ecs/Ecs.h"
#endif


namespace engine
{
	class Device;
	class CommandList;
	class TextureRTV;
	class Camera;
	class DepthPyramid;
	struct GpuShape;
	class LightData;
	class Ecs;

	class EcsDemo
	{
	public:
		EcsDemo(Device& device);

		void render(
			CommandList& cmd,
			TextureRTV rtv,
			TextureDSV dsv,
			const Camera& camera,
			const Matrix4f& jitterViewProjection);

	private:
		Device& m_device;
		
#ifdef ECSTEST22
		engine::Pipeline<shaders::RenderCubes> m_renderCubes;

		BufferOwner m_cpuPositions;
		BufferSRVOwner m_cpuPositionsSRV;

		ecs::Ecs m_ecs;
		uint8_t* m_ecsTransformData;
#endif
	};
}
