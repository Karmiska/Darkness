#include "Application.h"
#include "platform/window/Window.h"
#include "MainMenu.h"
#include "ui/MessageProcessor.h"
#include "ui/UiLayout.h"
#include "EngineView.h"
#include "EngineEventWrapper.h"
#include "Hierarchy.h"
#include "Browser.h"
#include "Inspector.h"
#include "platform/Environment.h"

#include "test/Test1.h"

#include <thread>
#include <chrono>

namespace editor
{
	Application::Application()
		: Frame(1280, 1024, GraphicsApi::DX12)
		, m_layoutSettingsManager{ engine::make_shared<ui::UiLayoutSettingsManager>(
			engine::pathClean(engine::pathJoin(engine::pathJoin(engine::pathExtractFolder(engine::getExecutableDirectory()), "../../../data/"), "LayoutSettings.json"))) }
		, m_scene{ nullptr }
		, m_mainMenu{ engine::make_shared<MainMenu>(this) }
		, m_verticalMenuMainLayout{ engine::make_shared<ui::UiLayout>(this, m_layoutSettingsManager->settings("verticalMenuMainLayout"), ui::UiLayoutDirection::Vertical)}
		, m_verticalMainLayout{ engine::make_shared<ui::UiLayout>(this, m_layoutSettingsManager->settings("verticalMainLayout"), ui::UiLayoutDirection::Vertical) }
		, m_horizontalTopLayout{ engine::make_shared<ui::UiLayout>(this, m_layoutSettingsManager->settings("horizontalTopLayout"), ui::UiLayoutDirection::Horizontal) }
		, m_engineViewDX12{ engine::make_shared<EngineView>(this, GraphicsApi::DX12, m_scene, "Scene")}
		//, m_engineViewVulkan{ engine::make_shared<EngineView>(this, GraphicsApi::Vulkan, m_scene, "Vulkan Scene")}
		, m_hierarchy{ engine::make_shared<Hierarchy>(this, "Hierarchy") }
		, m_browser{ engine::make_shared<Browser>(this, "Browser") }
		, m_inspector{ engine::make_shared<Inspector>(this, "Inspector") }
		//, m_hierarchy2{ engine::make_shared<Hierarchy>(this, "Hierarchy Vulkan") }
		//, m_engineViewInMainWindow{ true }
	{
		//m_scene->loadFrom("C:\\Users\\aleks\\Documents\\TestDarknessProject\\content\\scenes\\park");
		//m_hierarchy->forceRootFrame(true, GraphicsApi::DX12);
		//m_hierarchy2->forceRootFrame(true, GraphicsApi::Vulkan);

		//m_engineViewDX12->eventWrapper()->setDuplicateEventReceiver(m_engineViewVulkan->eventWrapper());
		//m_engineViewVulkan->eventWrapper()->setDuplicateEventReceiver(m_engineViewDX12->eventWrapper());

		backgroundColor(THEME.color("application_backgroundColor").xyz());

		addChild(m_verticalMenuMainLayout);
		m_verticalMenuMainLayout->forceObjectSize(0, m_mainMenu->height());
		m_verticalMenuMainLayout->addChild(m_mainMenu);
		m_verticalMenuMainLayout->addChild(m_verticalMainLayout);
		addAnchor(ui::UiAnchor{ m_verticalMenuMainLayout.get(), ui::AnchorType::TopLeft, ui::AnchorType::TopLeft});
		addAnchor(ui::UiAnchor{ m_verticalMenuMainLayout.get(), ui::AnchorType::BottomRight, ui::AnchorType::BottomRight});

		m_verticalMainLayout->addChild(m_horizontalTopLayout);
		m_verticalMainLayout->addChild(m_browser);
		//
		m_horizontalTopLayout->addChild(m_hierarchy);
		m_horizontalTopLayout->addChild(m_engineViewDX12);
		m_horizontalTopLayout->addChild(m_inspector);
		//addChild(m_hierarchy2);

		
		//addChild(m_engineViewVulkan);
	}

	void Application::frameMessageLoopClose()
	{
		removeChild(m_verticalMenuMainLayout);
		//removeChild(m_engineViewVulkan);
		//m_engineViewDX12 = nullptr;
		//m_engineViewVulkan = nullptr;

		//RenderTargets.clear();
		Frame::frameMessageLoopClose();
	}

	int Application::execute()
	{
		while (ui::MessageProcessor::instance().processMessages())
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
			invalidate();

			for (auto&& externWindow : m_externalFrames)
				externWindow->invalidate();

			//if(m_engineViewDX12)
			//	m_engineViewDX12->update();
			//if(m_engineViewVulkan)
			//	m_engineViewVulkan->update();
			//
			//if(m_engineViewVulkan)
			//	m_engineViewVulkan->setCameraTransform(m_engineViewDX12->getCameraTransform());
		}
		return 0;
	}

	void Application::onMouseDown(engine::MouseButton /*button*/, int /*x*/, int /*y*/)
	{
		/*if (m_engineViewInMainWindow)
		{
			auto newFrame = engine::make_shared<ui::Frame>(500, 500, GraphicsApi::Vulkan);
			m_externalFrames.emplace_back(newFrame);
			m_engineView->reparent(newFrame.get());
			m_engineView->area().position(0, 0);
			m_engineView->canMove(false);
			m_engineViewInMainWindow = false;
		}
		else
		{
			m_engineView->reparent(this);
			m_externalFrames.erase(m_externalFrames.end()-1);
			m_engineView->canMove(true);
			m_engineViewInMainWindow = true;
		}*/
	}
}
