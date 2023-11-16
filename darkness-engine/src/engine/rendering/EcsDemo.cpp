#include "engine/rendering/EcsDemo.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/rendering/DepthPyramid.h"
#include "engine/rendering/LightData.h"
#include "components/Camera.h"
#include "engine/rendering/ShapeMeshFactory.h"

#include <algorithm>

#if 1
#include <random>
std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(-130.0f, 130.0f);
float randomFloat()
{
	return static_cast<float>(distribution(generator));
}
#endif

namespace engine
{
#ifdef ECSTEST22
	class GravityWell
	{
	public:
		GravityWell()
			: position{ 0.0f, 0.0f, 0.0f, 1.0f }
			, mass{ 1000000.0f }
		{};
		Vector4f position;
		float mass;
	};

	class EcsTransform
	{
	public:
		EcsTransform()
			: position{ randomFloat(), randomFloat(), randomFloat(), 1.0f }
		{}
		Vector4f position;
	};

	class EcsRigidBody
	{
	public:
		Vector3f initialvelocity;
		Vector3f velocity;
		float mass;

		EcsRigidBody()
			: initialvelocity{ randomFloat() / 300.0f, randomFloat() / 300.0f, randomFloat() / 300.0f }
			, mass{ 1.0f }
			, velocity{ initialvelocity }
		{}

		void simulate(GravityWell& gravityWell, EcsTransform& transform, float timeStepMs)
		{
			auto towardsWell = (gravityWell.position.xyz() - transform.position.xyz()).normalize();
			float r = (gravityWell.position.xyz() - transform.position.xyz()).magnitude();
			
			float f = std::min((gravityWell.mass * mass) / (r * r * r), 0.01f);
			f = std::max(f, 0.0000001f);

			velocity += towardsWell * (f * (timeStepMs / 1000.0f));
			velocity *= 0.995f;
			transform.position += Vector4f(velocity, 0.0f);
		};
	};
#endif

	EcsDemo::EcsDemo(Device& device)
		: m_device{ device }
#ifdef ECSTEST22
		, m_renderCubes{ device.createPipeline<shaders::RenderCubes>() }
#endif
	{
#ifdef ECSTEST22
		constexpr size_t EcsTestEntityCount = 3000000;
		m_cpuPositions = device.createBuffer(BufferDescription()
			.elements(EcsTestEntityCount)
			.elementSize(sizeof(EcsTransform))
			.structured(true)
			.usage(ResourceUsage::Upload)
			.name("Ecs demo transforms"));
		m_cpuPositionsSRV = device.createBufferSRV(m_cpuPositions);
		m_ecsTransformData = static_cast<uint8_t*>(m_cpuPositions.resource().map(device));

		for (int i = 0; i < EcsTestEntityCount; ++i)
		{
			auto entity = m_ecs.createEntity();
			entity.addComponents<EcsTransform, EcsRigidBody>();
			//entity.addComponent<EcsTransform>();
			//entity.addComponent<EcsRigidBody>();
		}

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

		m_renderCubes.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_renderCubes.setRasterizerState(RasterizerDescription().cullMode(CullMode::None)); // TODO
		m_renderCubes.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));
#endif
	}

	void EcsDemo::render(
		CommandList& cmd,
		TextureRTV rtv,
		TextureDSV dsv,
		const Camera& camera,
		const Matrix4f& jitterViewProjection)
	{
#ifdef ECSTEST22
		uint32_t numObjects = 3000000;
		{
			CPU_MARKER(cmd.api(), "Simulate ECS entities");
			GravityWell gravityWell;
			m_ecs.query([&gravityWell](EcsTransform& transform, EcsRigidBody& rigidBody)
			{
				//EcsTransform* ptrA = &transform;
				//EcsRigidBody* ptrB = &rigidBody;
				//
				//for (int i = 0; i < VectorizationSize; ++i)
				//{
				//	ptrB->simulate(gravityWell, *ptrA, 1000.0f / 60.0f);
				//	++ptrA;
				//	++ptrB;
				//}
				rigidBody.simulate(gravityWell, transform, 1000.0f / 60.0f);
			});

			//auto& transformSystem = m_ecs.system<EcsTransform>();
			//numObjects = static_cast<uint32_t>(transformSystem.size());
			//memcpy(m_ecsTransformData, transformSystem.data(), sizeof(EcsTransform) * numObjects);
			m_ecs.copyRaw<EcsTransform>(reinterpret_cast<EcsTransform*>(m_ecsTransformData), numObjects);
		}
		{
			CPU_MARKER(cmd.api(), "Render ECS demo");
			GPU_MARKER(cmd, "Render ECS demo");

			cmd.setRenderTargets({ rtv }, dsv);

			m_renderCubes.vs.jitterViewProjectionMatrix = fromMatrix(jitterViewProjection * camera.projectionMatrix() * camera.viewMatrix());
			m_renderCubes.vs.transforms = m_cpuPositionsSRV;
			m_renderCubes.ps.color = { 1.0f, 0.0f, 0.0f, 1.0f };

			cmd.bindPipe(m_renderCubes);
			cmd.draw(36 * numObjects);

		}
#endif
	}

}
