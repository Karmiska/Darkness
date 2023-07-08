#include "test/Test1.h"

namespace editor
{
	Frame1::Frame1(Frame* parent)
		: Frame{ 200, 200, parent->api(), parent }
	{
		backgroundColor(engine::Vector3f{1.0f, 0.0f, 0.0f});
		themeSet(true);
	}

	Frame2::Frame2(Frame* parent)
		: Frame{ 200, 22, parent->api(), parent }
	{
		backgroundColor(engine::Vector3f{ 0.0f, 1.0f, 0.0f });
	}

	Frame3::Frame3(Frame* parent)
		: Frame{ 200, 22, parent->api(), parent }
	{
		backgroundColor(engine::Vector3f{ 0.0f, 0.0f, 1.0f });
	}
}
