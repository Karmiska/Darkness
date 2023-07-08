#pragma once

#include "tools/AtlasAllocator.h"
#include "engine/graphics/Device.h"

namespace engine
{
	class FontAtlas : public AtlasAllocator
	{
	public:
		FontAtlas(
			Device& device,
			int cellWidth,
			int cellHeight,
			int initialWidth = -1,
			int initialHeight = -1);
		TextureSRV atlas();

	protected:
		void OnAtlasSizeChange(int width, int height) override;
	private:
		Device& m_device;
		TextureUAVOwner m_atlasUAV;
		TextureSRVOwner m_atlasSRV;
	};
}
