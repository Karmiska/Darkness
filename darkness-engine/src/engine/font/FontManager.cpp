#include "engine/font/FontManager.h"
#include "engine/font/Font.h"


#include "engine/graphics/Resources.h"
#include "engine/graphics/CommandList.h"
#include "containers/unordered_map.h"
#include "tools/AtlasAllocator.h"

namespace engine
{
	FontManager::FontManager(Device& device)
		: m_device{ device }
		, m_library{ nullptr }
	{
		auto error = FT_Init_FreeType(&m_library);
		if (error)
		{
			LOG_ERROR("FreeType failed to initialize");
		}
	}

	TextureSRV FontManager::fontTexture()
	{
		return m_fonts[0]->fontAtlas();
	}

	FontManager::~FontManager()
	{
		m_fonts.clear();
		FT_Done_FreeType(m_library);
	}

	engine::shared_ptr<Font> FontManager::loadFont(const engine::string& fontPath)
	{
		m_fonts.emplace_back(engine::make_shared<Font>(m_device, m_library, fontPath));
		return m_fonts.back();
	}
}
