#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/ResourceOwners.h"
#include "containers/string.h"
#include "containers/memory.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace engine
{
	class Font;

	class FontManager
	{
	public:
		FontManager(Device& device);
		~FontManager();

		engine::shared_ptr<Font> loadFont(const engine::string& fontPath);

		TextureSRV fontTexture();
	private:
		Device& m_device;
		FT_Library m_library;
		engine::vector<engine::shared_ptr<Font>> m_fonts;
	};
}
