#include "MainMenu.h"
#include "engine/graphics/Device.h"
#include "ui/Theme.h"
#include "ui/Button.h"

namespace editor
{
	MainMenu::MainMenu(Frame* parent)
		: Frame{ parent->area().width(), 22, parent->api(), parent }
		, m_fileButton{ engine::make_shared<ui::Button>(this, "File") }
	{
		canMove(ui::AllowedMovement::None);
		backgroundColor(ui::Theme::instance().color("mainmenu_backgroundColor").xyz());
		area().position(0, 0);
		addChild(m_fileButton);
		m_fileButton->canMove(ui::AllowedMovement::None);
	}

}