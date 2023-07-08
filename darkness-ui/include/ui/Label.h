#pragma once

#include "ui/Frame.h"
#include "containers/string.h"

namespace ui
{
	class DrawCommandBuffer;

	class Label : public ui::Frame
	{
	public:
		Label(Frame* parent, const engine::string& text);

	protected:
		void onPaint(DrawCommandBuffer& cmd) override;

	private:
		engine::string m_text;
	};
}
