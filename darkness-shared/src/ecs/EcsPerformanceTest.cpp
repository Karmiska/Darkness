#include "ecs/EcsPerformanceTest.h"

#include "ecs/Ecs.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"
#include "tools/Debug.h"

using namespace engine;

namespace ecs
{
#undef INITIALIZE_COMPONENTS

	class GravityWell
	{
	public:
#ifdef INITIALIZE_COMPONENTS
		GravityWell()
			: position{ 0.0f, 0.0f, 0.0f, 1.0f }
			, mass{ 1000000.0f }
		{};
#endif
		Vector4f position;
		float mass;
	};

	class EcsTransform
	{
	public:
#ifdef INITIALIZE_COMPONENTS
		//EcsTransform() = default;
		Vector4f position = { 1.0f, 1.0f, 1.0f, 1.0f };
#else
		Vector4f position;
#endif

		EcsTransform operator+(const EcsTransform& tr)
		{
			auto newPos = position + tr.position;
			return EcsTransform{ newPos };
		}

		EcsTransform& operator+=(const EcsTransform& tr)
		{
			position += tr.position;
			return *this;
		}
	};

	class EcsRigidBody
	{
	public:
#ifdef INITIALIZE_COMPONENTS
		EcsRigidBody() = default;
		Vector3f initialvelocity = { 1.0f, 1.0f, 1.0f };
		Vector3f velocity = { 1.0f, 1.0f, 1.0f };
		float mass = 1.0f;
#else
		Vector3f initialvelocity;
		Vector3f velocity;
		float mass;
#endif
		void simulate(GravityWell& gravityWell, EcsTransform& transform, float timeStepMs)
		{
			auto towardsWell = (gravityWell.position.xyz() - transform.position.xyz()).normalize();
			float r = (gravityWell.position.xyz() - transform.position.xyz()).magnitude();

			float f = 1.0f;// std::min((gravityWell.mass * mass) / (r * r * r), 0.01f);
			//f = std::max(f, 0.0000001f);

			velocity += towardsWell * (f * (timeStepMs / 1000.0f));
			velocity *= 0.995f;
			transform.position += Vector4f(velocity, 0.0f);
		};
	};

	EcsPerformanceTest::EcsPerformanceTest()
	{
		QueryPerformanceFrequency(&freq);
	}

    void EcsPerformanceTest::runTinyTest()
    {
		performTest(100);

		// Testing with : 1000000 entities
		// Prewarming took : 0.000000 ms
		// Populating took : 9.339600 ms
		// Simulating took : 4.313100 ms
		// Combined : 13.652700 ms
    }

    void EcsPerformanceTest::runSmallTest()
    {
		performTest(100'000'000);

		// Testing with : 100000000 entities
		// Prewarming took : 0.000000 ms
		// Populating took : 379.079895 ms
		// Simulating took : 348.954803 ms
		// Combined : 728.034729 ms

		// INITIALIZE_COMPONENTS
		// Testing with : 100000000 entities
		// Prewarming took : 0.000000 ms
		// Populating took : 939.110107 ms
		// Simulating took : 233.944000 ms
		// Combined : 1173.054077 ms
    }

    void EcsPerformanceTest::runLargeTest()
    {
		performTest(1000'000'000);

		// Testing with : 1000000000 entities
		// Prewarming took : 0.000000 ms
		// Populating took : 3785.807373 ms
		// Simulating took : 3305.108643 ms
		// Combined : 7090.916016 ms

		// Testing with : 1000000000 entities
		// Prewarming took : 0.000000 ms
		// Populating took : 8810.738281 ms
		// Simulating took : 2272.018311 ms
		// Combined : 11082.756836 ms
    }

	void EcsPerformanceTest::performTest(size_t count)
	{
		ecs::Ecs ecs;

		auto archeType = ecs.archeType<EcsTransform, EcsRigidBody>();
		auto archeTypeId = archeType.id();

		QueryPerformanceCounter(&prewarm);
		engine::vector<ecs::EntityId> entities;
		entities.resize(count);

		QueryPerformanceCounter(&start);
		for (int i = 0; i < count; ++i)
		{
			auto entity = ecs.createEntity();
			entities[i] = entity.entityId;

			entity.setComponents(archeTypeId);
		}
		QueryPerformanceCounter(&populated);

		GravityWell gravityWell;
		ecs.query([&gravityWell, &ecs](EcsTransform& transform, EcsRigidBody& rigidBody)
		{
			rigidBody.simulate(gravityWell, transform, 1000.0f / 60.0f);
		});

		QueryPerformanceCounter(&simulated);

		for (auto&& entity : entities)
			ecs.destroyEntity(entity);

		QueryPerformanceCounter(&erased);

		printResults(count);
	}

	void EcsPerformanceTest::printResults(size_t count) const
	{
		LOG_PURE("Testing with: %zu entities", count);

		LARGE_INTEGER measure;
		measure.QuadPart = start.QuadPart - prewarm.QuadPart;
		measure.QuadPart *= 1000;
		LOG_PURE("Prewarming took: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));

		measure.QuadPart = populated.QuadPart - start.QuadPart;
		measure.QuadPart *= 1000;
		LOG_PURE("Populating took: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));

		measure.QuadPart = simulated.QuadPart - populated.QuadPart;
		measure.QuadPart *= 1000;
		LOG_PURE("Simulating took: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));

		measure.QuadPart = erased.QuadPart - simulated.QuadPart;
		measure.QuadPart *= 1000;
		LOG_PURE("Erasing: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));

		measure.QuadPart = erased.QuadPart - prewarm.QuadPart;
		measure.QuadPart *= 1000;
		LOG_PURE("Combined: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));
	}
}
