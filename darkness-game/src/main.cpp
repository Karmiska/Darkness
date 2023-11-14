#define ECSTEST2

#ifndef ECSTEST2
#include "AllocatorHooks.h"

#include "platform/Platform.h"
#include "application/Application.h"
using namespace application;
//#include "plugins/PluginManager.h"
#endif


#ifdef ECSTEST2
#include <Windows.h>
#undef min
#undef max
#include <cmath>

//constexpr size_t DataCount = 100'000'000;
constexpr size_t DataCount = 10'000'000;

#include "engine/Ecs.h"
#include "ecs/Ecs.h"
#include "ecs/TypeSort.h"

#include <xmmintrin.h>

class Vector3f
{
public:
	Vector3f(float _x, float _y, float _z)
		: x{ _x }
		, y{ _y }
		, z{ _z }
	{}

	Vector3f& operator+=(const Vector3f& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3f operator+(const Vector3f& v) const
	{
		Vector3f tmp = *this;
		tmp.x += v.x;
		tmp.y += v.y;
		tmp.z += v.z;
		return tmp;
	}

	Vector3f operator-(const Vector3f& v) const
	{
		Vector3f tmp = *this;
		tmp.x -= v.x;
		tmp.y -= v.y;
		tmp.z -= v.z;
		return tmp;
	}

	Vector3f operator*(float v) const
	{
		Vector3f tmp = *this;
		tmp.x *= v;
		tmp.y *= v;
		tmp.z *= v;
		return tmp;
	}

	Vector3f& operator*=(float v)
	{
		x += v;
		y += v;
		z += v;
		return *this;
	}

	float magnitude() const
	{
		double xx = (double)x * (double)x;
		double yy = (double)y * (double)y;
		double zz = (double)z * (double)z;
		double sum = xx + yy + zz;
		if (sum == 0.0)
		{
			return static_cast<float>(0.0);
		}
		else
			return static_cast<float>(std::sqrt(sum));
	}

	Vector3f& normalize()
	{
		double xx = (double)x;
		double yy = (double)y;
		double zz = (double)z;
		double mag = std::sqrt(xx * xx + yy * yy + zz * zz);
		if (mag != 0.0f)
		{
			x = (float)(x / mag);
			y = (float)(y / mag);
			z = (float)(z / mag);
		}
		return *this;
	}

	float x, y, z;
};

class Vector4f
{
public:
	Vector4f(float _x, float _y, float _z, float _w)
		: x{ _x }
		, y{ _y }
		, z{ _z }
		, w{ _w }
	{}

	Vector4f(Vector3f vec, float w)
		: x{ vec.x }
		, y{ vec.y }
		, z{ vec.z }
		, w{ w }
	{}

	Vector4f& operator+=(const Vector4f& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	Vector4f operator+(const Vector4f& v) const
	{
		Vector4f tmp = *this;
		tmp.x += v.x;
		tmp.y += v.y;
		tmp.z += v.z;
		tmp.w += v.w;
		return tmp;
	}

	Vector3f xyz() const
	{
		return { x, y, z };
	}

	float x, y, z, w;
};

#endif


using namespace engine;

#ifdef ECSTEST2

#include <random>
std::default_random_engine agenerator;
std::uniform_real_distribution<double> adistribution(-130.0f, 130.0f);
float arandomFloat()
{
	return static_cast<float>(adistribution(agenerator));
}

std::default_random_engine igenerator;
std::uniform_int_distribution<uint64_t> idistribution(0, DataCount-1);
uint64_t arandomUInt()
{
	return static_cast<uint64_t>(idistribution(igenerator));
}

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
		: position{ arandomFloat(), arandomFloat(), arandomFloat(), 1.0f }
	{}

	EcsTransform(const Vector4f& _position)
		: position{ _position }
	{}

	EcsTransform operator+(const EcsTransform& tr)
	{
		auto newPos = position + tr.position;
		return EcsTransform{newPos};
	}

	EcsTransform& operator+=(const EcsTransform& tr)
	{
		position += tr.position;
		return *this;
	}

	Vector4f position;
};

class EcsRigidBody
{
public:
	Vector3f initialvelocity;
	Vector3f velocity;
	float mass;

	EcsRigidBody()
		: initialvelocity{ arandomFloat() / 300.0f, arandomFloat() / 300.0f, arandomFloat() / 300.0f }
		, mass{ 1.0f }
		, velocity{ initialvelocity }
	{}

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

struct A
{
	bool operator == (const A& other) const = default;
	int32_t value = 0;
};

struct B
{
	bool operator == (const B& other) const = default;
	int32_t value = 0;
};

#endif

int doWork();

#if defined(_WIN32) && !defined(_DURANGO)
int CALLBACK WinMain(
	_In_ HINSTANCE, // hInstance,
	_In_ HINSTANCE,  // hPrevInstance,
	_In_ LPSTR, //     lpCmdLine,
	_In_ int   //       nCmdShow
)
{
	return doWork();
}

int main(int argc, char* argv[])
{
	return doWork();
}

#ifdef ECSTEST2
//struct Ent
//{
//	Ent()
//		: archeType{ ecs::InvalidArchetypeId }
//	{}
//
//	ecs::ComponentArcheTypeId archeType;
//
//	template<typename T>
//	void addComponent()
//	{
//		if(archeType == ecs::InvalidArchetypeId)
//			archeType = ecs::ComponentTypeStorage::archetypeId<T>();
//		else
//		{
//			auto tmp = ecs::ComponentTypeStorage::archeTypeInfo(archeType);
//			LOG("ping");
//		}
//		
//	};
//};
#endif

int doWork()
{
#ifndef ECSTEST2
    Application app;

#else
	
	//struct a {
	//};
	//struct b {
	//};
	//struct c {
	//};
	//
	//auto tidA = ecs::ComponentTypeStorage::typeId<a>();
	//auto tidB = ecs::ComponentTypeStorage::typeId<b>();
	//auto tidC = ecs::ComponentTypeStorage::typeId<c>();
	//
	//auto t1 = ecs::ComponentTypeStorage::archeTypeId({ tidA });
	//auto t2 = ecs::ComponentTypeStorage::archeTypeId({ tidB });
	//auto t3 = ecs::ComponentTypeStorage::archeTypeId({ tidA, tidB });
	//auto t4 = ecs::ComponentTypeStorage::archeTypeId({ tidB, tidA });
	//auto t5 = ecs::ComponentTypeStorage::archeTypeId({ tidC });
	//auto t6 = ecs::ComponentTypeStorage::archeTypeId({ tidA, tidB, tidC });
	//auto t7 = ecs::ComponentTypeStorage::archeTypeId({ tidA, tidC, tidB });
	//auto t8 = ecs::ComponentTypeStorage::archeTypeId({ tidB, tidA, tidC });
	//auto t9 = ecs::ComponentTypeStorage::archeTypeId({ tidB, tidC, tidA });
	//auto t10 = ecs::ComponentTypeStorage::archeTypeId({ tidC, tidA, tidB });
	//auto t11 = ecs::ComponentTypeStorage::archeTypeId({ tidC, tidB, tidA });
	//
	//LOG("[%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u", 
	//	t1.id(), t2.id(), t3.id(), t4.id(), t5.id(), t6.id(), t7.id(), t8.id(), t9.id(), t10.id(), t11.id());
	//
	//
	

	//auto t1 = ecs::ComponentTypeStorage::archetypeId<a>();
	//auto t2 = ecs::ComponentTypeStorage::archetypeId<b>();
	//auto t3 = ecs::ComponentTypeStorage::archetypeId<a, b>();
	//auto t4 = ecs::ComponentTypeStorage::archetypeId<b, a>();
	//auto t5 = ecs::ComponentTypeStorage::archetypeId<c>();
	//auto t6 = ecs::ComponentTypeStorage::archetypeId<a, b, c>();
	//auto t7 = ecs::ComponentTypeStorage::archetypeId<a, c, b>();
	//auto t8 = ecs::ComponentTypeStorage::archetypeId<b, a, c>();
	//auto t9 = ecs::ComponentTypeStorage::archetypeId<b, c, a>();
	//auto t10 = ecs::ComponentTypeStorage::archetypeId<c, a, b>();
	//auto t11 = ecs::ComponentTypeStorage::archetypeId<c, b, a>();
	//
	//LOG("[%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u", 
	//	t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11);
	//
	//auto temp = ecs::ComponentTypeStorage::typeInfo(
	//	ecs::ComponentTypeStorage::typeId<int>());
	//auto temp2 = temp.create(10);
	//decltype(temp2) a;
	//
	//Ent ent;
	//ent.addComponent<int>();
	//ent.addComponent<float>();

	//constexpr size_t DataCount = 100;

	{
		ecs::Ecs ecs;

		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		LARGE_INTEGER start;
		QueryPerformanceCounter(&start);
		for (int i = 0; i < DataCount; ++i)
		{
			auto entity = ecs.createEntity();
			entity.addComponent<EcsTransform>();
			entity.addComponent<EcsRigidBody>();
			entity.addComponent<A>();
			entity.addComponent<B>();
		}

		//for (int i = 0; i < DataCount; ++i)
		//{
		//	auto entity = ecs.createEntity();
		//	entity.addComponent<A>();
		//	entity.addComponent<B>();
		//	entity.addComponent<EcsTransform>();
		//	entity.addComponent<EcsRigidBody>();
		//}
		//ecs.refreshTypeAllocations<EcsTransform, EcsRigidBody, A, B>();

		LARGE_INTEGER populated;
		QueryPerformanceCounter(&populated);

#undef CALLCHECK
#ifdef CALLCHECK
		std::atomic<int> called = 0;
#endif

		GravityWell gravityWell;
		ecs.query([&gravityWell, &ecs](EcsTransform& transform, EcsRigidBody& rigidBody)
			{
				//EcsTransform& randomEntityTransform = ecs.query<EcsTransform>(ecs::EntityId{ arandomUInt() });

				//transform += randomEntityTransform;
				rigidBody.simulate(gravityWell, transform, 1000.0f / 60.0f);
			});

		LARGE_INTEGER simulated;
		QueryPerformanceCounter(&simulated);
#ifdef CALLCHECK
		LOG("Called: %i times", called.load());
#endif

		LOG("Testing with: %zu entities", DataCount);

		LARGE_INTEGER measure;
		measure.QuadPart = populated.QuadPart - start.QuadPart;
		measure.QuadPart *= 1000;
		LOG("Populating took: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));

		measure.QuadPart = simulated.QuadPart - populated.QuadPart;
		measure.QuadPart *= 1000;
		LOG("Simulating took: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));
	}

#if 0
	{
		ecs::Ecs ecs;

		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		LARGE_INTEGER start;
		QueryPerformanceCounter(&start);

		for (int i = 0; i < DataCount; ++i)
		{
			auto& entity = ecs.createEntity();
			entity.addComponent<A>();
			entity.addComponent<B>();
		}
		ecs.refreshTypeAllocations<A, B>();

		LARGE_INTEGER populated;
		QueryPerformanceCounter(&populated);

		//GravityWell gravityWell;
		//ecs.query([](A& a, B& b)
		//	{
		//
		//		for (int i = 0; i < 8; ++i)
		//		{
		//			A* ptrA = &a;
		//			B* ptrB = &b;
		//
		//			__m256i tempA = _mm256_loadu_si256(reinterpret_cast<__m256i*>(ptrA)); // = _mm256_set_epi32(2, 4, 6, 8, 10, 12, 14, 16);	// 256 - bit vector containing 8 floats
		//			__m256i tempB = _mm256_loadu_si256(reinterpret_cast<__m256i*>(ptrB));
		//			tempA = _mm256_add_epi32(tempA, tempB);
		//			memcpy(ptrA, &tempA, sizeof(__m256i));
		//
		//			ptrA += 8;
		//			ptrB += 8;
		//		}
		//		//for (int i = 0; i < VectorizationSize; ++i)
		//		//{
		//		//	ptrA->value += ptrB->value;
		//		//	++ptrA;
		//		//	++ptrB;
		//		//}
		//
		//		//a.value += b.value;
		//		//rigidBody.simulate(gravityWell, transform, 1000.0f / 60.0f);
		//	});

		GravityWell gravityWell;
		ecs.query([&gravityWell](EcsTransform& transform, EcsRigidBody& rigidBody)
			{
				rigidBody.simulate(gravityWell, transform, 1000.0f / 60.0f);
			});

		LARGE_INTEGER simulated;
		QueryPerformanceCounter(&simulated);


		LOG_PURE("Populated and simulated %u entities with two components", DataCount);
		LARGE_INTEGER measure;
		measure.QuadPart = populated.QuadPart - start.QuadPart;
		measure.QuadPart *= 1000;
		LOG_PURE("Populating took: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));

		measure.QuadPart = simulated.QuadPart - populated.QuadPart;
		measure.QuadPart *= 1000;
		LOG_PURE("Simulating took: %f ms", static_cast<float>(static_cast<double>(measure.QuadPart) / static_cast<double>(freq.QuadPart)));
	}
#endif
#endif

    return 0;
}
#endif

#if defined(_DURANGO)

#define NOMINMAX

//using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
//using namespace DirectX;

ref class ViewProvider sealed : public IFrameworkView
{
public:
	ViewProvider() :
		m_exit(false)
	{
	}

	// IFrameworkView methods
	virtual void Initialize(CoreApplicationView^ applicationView)
	{
		applicationView->Activated +=
			ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &ViewProvider::OnActivated);

		CoreApplication::Suspending +=
			ref new EventHandler<SuspendingEventArgs^>(this, &ViewProvider::OnSuspending);

		CoreApplication::Resuming +=
			ref new EventHandler<Platform::Object^>(this, &ViewProvider::OnResuming);

		CoreApplication::DisableKinectGpuReservation = true;
	}

	virtual void Uninitialize()
	{
	}

	virtual void SetWindow(CoreWindow^ window) 
	{
		window->Closed +=
			ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &ViewProvider::OnWindowClosed);

		m_window = engine::make_shared<platform::Window>(reinterpret_cast<void*>(window), 1920, 1080);
		m_engine = engine::make_unique<Engine>(m_window, ""); 
		m_engine->scene().loadFrom("C:/Users/aleksi.jokinen/Documents/DarknessDemo/content/scenes/ball");
	}

	virtual void Load(Platform::String^ entryPoint)
	{
	}

	virtual void Run()
	{
		while (!m_exit)
		{
			//m_game->Tick();

			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		}
	}

protected:
	// Event handlers
	void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
	{
		auto deferral = args->SuspendingOperation->GetDeferral();

		/*create_task([this, deferral]()
		{
			//m_game->OnSuspending();

			deferral->Complete();
		});*/
	}

	void OnResuming(Platform::Object^ sender, Platform::Object^ args)
	{
		//m_game->OnResuming();
	}

	void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
	{
		m_exit = true;
	}

private:
	bool                    m_exit;
	engine::shared_ptr<platform::Window> m_window;
	engine::unique_ptr<Engine>   m_engine;
};

ref class ViewProviderFactory : IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView()
	{
		return ref new ViewProvider();
	}
};

[Platform::MTAThread]
int __cdecl main(Platform::Array<Platform::String^>^ /*argv*/)
{
	auto viewProviderFactory = ref new ViewProviderFactory();
	CoreApplication::Run(viewProviderFactory);
	return 0;
}

#endif
