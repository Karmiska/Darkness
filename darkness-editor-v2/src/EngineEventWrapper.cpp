#include "EngineEventWrapper.h"
#include "engine/graphics/Device.h"

#define ENGINE_ENABLED

namespace editor
{
	//engine::vector<TextureSRVOwner> RenderTargets;

	EngineEventWrapper::EngineEventWrapper(Frame* parent, GraphicsApi api, engine::shared_ptr<engine::Scene> scene)
		: Frame{ 1024 - 6, 768 - 24, parent->api(), parent }
#ifdef ENGINE_ENABLED
		, m_engine{ nullptr }
#endif
		, m_lastPosition{ 0, 0 }
		, m_receiver{ nullptr }
		, m_inMouseMove{ false }
		, m_inMouseDown{ false }
		, m_inMouseUp{ false }
		, m_inMouseDoubleClick{ false }
		, m_inMouseWheel{ false }
		, m_inKeyDown{ false }
		, m_inKeyUp{ false }
	{
		canMove(ui::AllowedMovement::None);
		forceRootFrame(true, parent->api());
		drawBackground(false);
		//backgroundColor({ 1.0f, 0.0f, 0.0f });
		
#ifdef ENGINE_ENABLED
		if (api == GraphicsApi::Vulkan)
		{
			m_engine = engine::make_unique<Engine>(this->windowShared(), "C:\\Users\\Aleksi Jokinen\\Darkness\\shaders\\", api, EngineMode::OwnThreadNoPresent, "EngineWrapper", scene);
		}

		if (api == GraphicsApi::DX12)
		{
#ifdef SINGLE_THREADED_ENGINE_FRAME
			m_engine = engine::make_unique<Engine>(this->windowShared(), "C:\\Users\\Aleksi Jokinen\\Darkness\\shaders\\", api, EngineMode::Passive, "EngineWrapper", scene);
#else
			m_engine = engine::make_unique<Engine>(this->windowShared(), "C:\\Users\\Aleksi Jokinen\\Darkness\\shaders\\", api, EngineMode::OwnThread, "EngineWrapper", scene);
#endif
		}

		m_engine->loadScene("C:\\Users\\aleks\\Documents\\TestDarknessProject\\content\\scenes\\ball2");
#endif
	}

	void EngineEventWrapper::setDuplicateEventReceiver(ui::UiEvents* receiver)
	{
		m_receiver = receiver;
	}

	void EngineEventWrapper::onFrameWindowChangeBegin()
	{
#ifdef ENGINE_ENABLED
		if (m_engine)
			m_engine->moveToNewWindowBegin();
#endif
	}

	void EngineEventWrapper::onFrameWindowChangeEnd(engine::shared_ptr<platform::Window> newWindow)
	{
		//m_engine = engine::make_unique<Engine>(windowShared(), "C:\\Users\\Aleksi Jokinen\\Darkness\\shaders\\");
		//m_engine->loadScene("C:\\Users\\Aleksi Jokinen\\Darkness\\content\\scenes\\clusterterrain");
#ifdef ENGINE_ENABLED
		if (m_engine)
			m_engine->moveToNewWindowEnd(newWindow);
#endif
	}

	void EngineEventWrapper::onMouseMove(int x, int y)
	{
		if (m_inMouseMove)
			return;
		m_inMouseMove = true;
#ifdef ENGINE_ENABLED
		m_engine->onMouseMove(x, y);
#endif
		if(m_receiver)
			m_receiver->onMouseMove(x, y);
		m_inMouseMove = false;
	}

	void EngineEventWrapper::onMouseDown(engine::MouseButton button, int x, int y)
	{
		if (m_inMouseDown)
			return;
		m_inMouseDown = true;
#ifdef ENGINE_ENABLED
		m_engine->cameraInputActive(true);
		m_engine->onMouseDown(button, x, y);
#endif

		if (m_receiver)
			m_receiver->onMouseDown(button, x, y);
		m_inMouseDown = false;
	}

	void EngineEventWrapper::onMouseUp(engine::MouseButton button, int x, int y)
	{
		if (m_inMouseUp)
			return;
		m_inMouseUp = true;
#ifdef ENGINE_ENABLED
		m_engine->onMouseUp(button, x, y);
		m_engine->cameraInputActive(false);
#endif

		if (m_receiver)
			m_receiver->onMouseUp(button, x, y);
		m_inMouseUp = false;
	}

	void EngineEventWrapper::onMouseDoubleClick(engine::MouseButton button, int x, int y)
	{
		if (m_inMouseDoubleClick)
			return;
		m_inMouseDoubleClick = true;
#ifdef ENGINE_ENABLED
		m_engine->onMouseDoubleClick(button, x, y);
#endif

		if (m_receiver)
			m_receiver->onMouseDoubleClick(button, x, y);
		m_inMouseDoubleClick = false;
	}

	void EngineEventWrapper::onMouseWheel(int x, int y, int delta)
	{
		if (m_inMouseWheel)
			return;
		m_inMouseWheel = true;
#ifdef ENGINE_ENABLED
		m_engine->onMouseWheel(x, y, delta);
#endif

		if (m_receiver)
			m_receiver->onMouseWheel(x, y, delta);
		m_inMouseWheel = false;
	}

	void EngineEventWrapper::onKeyDown(engine::Key key, const engine::ModifierState& modifierState)
	{
		if (m_inKeyDown)
			return;
		m_inKeyDown = true;
#ifdef ENGINE_ENABLED
		m_engine->onKeyDown(key, modifierState);
#endif

		if (m_receiver)
			m_receiver->onKeyDown(key, modifierState);
		m_inKeyDown = false;
	}

	void EngineEventWrapper::onKeyUp(engine::Key key, const engine::ModifierState& modifierState)
	{
		if (m_inKeyUp)
			return;
		m_inKeyUp = true;
#ifdef ENGINE_ENABLED
		m_engine->onKeyUp(key, modifierState);
#endif

		if (m_receiver)
			m_receiver->onKeyUp(key, modifierState);
		m_inKeyUp = false;
	}

	void EngineEventWrapper::update()
	{

	}

	CameraTransform EngineEventWrapper::getCameraTransform() const
	{
#ifdef ENGINE_ENABLED
		return m_engine->getCameraTransform();
#else
		return {};
#endif
	}

	void EngineEventWrapper::setCameraTransform(const CameraTransform& transform)
	{
#ifdef ENGINE_ENABLED
		m_engine->setCameraTransform(transform);
#endif
	}

	void EngineEventWrapper::onPaint(ui::DrawCommandBuffer& cmd)
	{
#ifdef ENGINE_ENABLED
		if (m_engine->mode() == EngineMode::OwnThreadNoPresent)
		{
			auto cpuRTV = m_engine->grabRTV();
			if (cpuRTV.pitchBytes == 0 || cpuRTV.data->size() == 0)
				return;

			if (!m_renderTarget ||
				m_renderTarget.width() != cpuRTV.width ||
				m_renderTarget.height() != cpuRTV.height)
			{
				auto parentRootFrame = getParentRootFrame();

				//if (m_renderTarget)
				//{
				//	for (auto rt = RenderTargets.begin(); rt != RenderTargets.end(); ++rt)
				//	{
				//		if ((*rt).resource() == m_renderTarget)
				//		{
				//			RenderTargets.erase(rt);
				//			break;
				//		}
				//	}
				//}

				//RenderTargets.emplace_back(parentRootFrame->getRootFrame()->rendering->device().loadTexture(cpuRTV));
				//m_renderTarget = RenderTargets.back();
				m_renderTarget = parentRootFrame->getRootFrame()->rendering->device().loadTexture(cpuRTV);
			}
			else
			{
				auto parentRootFrame = getParentRootFrame();
				parentRootFrame->getRootFrame()->rendering->device().copyTexture(cpuRTV, m_renderTarget);
			}

			cmd.drawImage(
				0, 0, width(), height(),
				m_renderTarget);
		}
		else if (m_engine->mode() == EngineMode::Passive)
		{
			m_engine->stepEngine();
			if(m_engine->hasStoredCommandList())
				cmd.executeCommandList(
					m_engine->renderSetup(),
					std::move(m_engine->storedCommandList()));
			
			cmd.drawImage(
				0, 0, width(), height(),
				m_engine->renderSetup()->currentRTVSRV());
		}
#endif
	}

	void EngineEventWrapper::onResize(int width, int height)
	{
#ifdef ENGINE_ENABLED
		m_engine->refreshSize();
#endif
	}
}