#pragma once

#include "ui/Frame.h"

namespace ui
{
	class Button;
}

namespace editor
{
	

	class MainMenu : public ui::Frame
	{
	public:
		MainMenu(Frame* parent);

	private:
		engine::shared_ptr<ui::Button> m_fileButton;
	};
}
