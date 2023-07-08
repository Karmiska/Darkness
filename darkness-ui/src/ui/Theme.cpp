#include "ui/Theme.h"
#include "tools/Settings.h"
#include "platform/Environment.h"
#include "tools/PathTools.h"

namespace ui
{
	void Theme::loadSettings(const engine::string& themeFilePath)
	{
		m_settings = engine::make_shared<tools::Settings>(themeFilePath);
	}

	engine::string Theme::image(const engine::string& assetName)
	{
		auto res = m_settings->get<engine::string>(assetName, "");
		auto repl = res.find("$DATA_ROOT");
		
		if (repl != engine::string::npos)
		{
			auto dataRoot = engine::pathClean(engine::pathJoin(engine::pathExtractFolder(engine::getExecutableDirectory()), "..\\..\\..\\data"));
			res.replace(repl, 10, dataRoot);
		}
		return res;
	}

	engine::Vector4f Theme::color(const engine::string& colorName)
	{
		return engine::Vector4f{
			m_settings->get<float>(colorName + "_r", 1.0f),
			m_settings->get<float>(colorName + "_g", 0.0f),
			m_settings->get<float>(colorName + "_b", 1.0f),
			m_settings->get<float>(colorName + "_a", 1.0f) };
	}
}
