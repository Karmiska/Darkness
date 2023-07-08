#include "ui/Label.h"
#include "ui/DrawCommandBuffer.h"

namespace ui
{
	Label::Label(Frame* parent, const engine::string& text)
		: Frame{ 200, 14, parent->api(), parent }
		, m_text{ text }
	{
		canReceiveMouseMessages(false);
		blocksMouseMessages(false);
		canMove(ui::AllowedMovement::None);
		backgroundColor(engine::Vector3f{ 1.0f, 0.0f, 0.0f });
		drawBackground(false);
	}

	void Label::onPaint(ui::DrawCommandBuffer& cmd)
	{
		cmd.drawText(0, 0, 200, 14, m_text);
	}
}