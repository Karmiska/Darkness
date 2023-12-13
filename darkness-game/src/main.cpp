#define ECSTEST2

#ifndef ECSTEST2
#include "AllocatorHooks.h"
#include "platform/Platform.h"
#include "application/Application.h"
#include "application/GrassSystem.h"

using namespace application;
using namespace engine;
#else
#include "ecs/EcsPerformanceTest.h"
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
#endif

int doWork()
{
#ifdef ECSTEST2
	ecs::EcsPerformanceTest ecsPerfTest;
	ecsPerfTest.runSmallTest();
#else
	//game::GrassSystem grassSystem(20, 20);
	//grassSystem.grow(60.0f * 60.0f * 24.0f * 30.0f); // 30 days

    //Application app;
#endif
    return 0;
}

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
