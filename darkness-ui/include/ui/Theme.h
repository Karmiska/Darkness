#pragma once

#include "engine/primitives/Vector4.h"
#include "containers/string.h"
#include "containers/memory.h"

namespace tools
{
	class Settings;
}

namespace ui
{
	class Theme
	{
	public:
		static Theme& instance()
		{
			static Theme theme;
			return theme;
		}
		void loadSettings(const engine::string& themeFilePath);
		
		engine::string image(const engine::string& assetName);
		engine::Vector4f color(const engine::string& colorName);
	private:
		Theme() {};
		engine::shared_ptr<tools::Settings> m_settings;
	};
}

#define THEME ui::Theme::instance()