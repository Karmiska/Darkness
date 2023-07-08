#pragma once

#include "ui/Frame.h"
#include "ui/Theme.h"
#include "containers/memory.h"
#include "containers/vector.h"
#include "engine/Scene.h"

namespace ui
{
	class UiLayout;
	class UiLayoutSettingsManager;
}

namespace editor
{
	class EngineView;
	class MainMenu;
	class Frame1;
	class Hierarchy;
	class Browser;
	class Inspector;

	class Application : public ui::Frame
	{
	public:
		Application();
		int execute();

	protected:
		void frameMessageLoopClose();

	protected:
		void onMouseDown(engine::MouseButton button, int x, int y) override;

	private:
		engine::shared_ptr<ui::UiLayoutSettingsManager> m_layoutSettingsManager;
		engine::shared_ptr<engine::Scene> m_scene;
		engine::shared_ptr<MainMenu> m_mainMenu;
		engine::shared_ptr<ui::UiLayout> m_verticalMenuMainLayout;
		engine::shared_ptr<ui::UiLayout> m_verticalMainLayout;
		engine::shared_ptr<ui::UiLayout> m_horizontalTopLayout;
		engine::shared_ptr<EngineView> m_engineViewDX12;
		engine::shared_ptr<EngineView> m_engineViewVulkan;
		engine::shared_ptr<Hierarchy> m_hierarchy;
		engine::shared_ptr<Hierarchy> m_hierarchy2;
		engine::shared_ptr<Browser> m_browser;
		engine::shared_ptr<Inspector> m_inspector;
		engine::shared_ptr<Frame1> m_test1;
		engine::vector<engine::shared_ptr<ui::Frame>> m_externalFrames;
		
		bool m_engineViewInMainWindow;
		
	};
}
