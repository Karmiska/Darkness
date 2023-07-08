#include "ui/Button.h"
#include "ui/Theme.h"
#include "ui/Label.h"

namespace ui
{
	Button::Button(Frame* parent, const engine::string& text)
		: Frame{ 200, 22, parent->api(), parent }
		, m_label{ engine::make_shared<Label>(this, text) }
	{
		backgroundColor(ui::Theme::instance().color("button_backgroundColor").xyz());
		addChild(m_label);
		m_label->position(0, 4);
	}

}