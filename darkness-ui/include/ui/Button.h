#pragma once

#include "ui/Frame.h"
#include "tools/image/Image.h"
#include "containers/string.h"

namespace ui
{
	class Label;

	class Button : public ui::Frame
	{
	public:
		Button(Frame* parent, const engine::string& text);

	private:
		engine::shared_ptr<Label> m_label;
	};
}
