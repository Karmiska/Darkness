#include "engine/font/FontAtlas.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
	FontAtlas::FontAtlas(
		Device& device,
		int width,
		int height,
		int initialWidth,
		int initialHeight)
		: AtlasAllocator{ width, height, initialWidth, initialHeight }
		, m_device{ device }
	{
		if ((initialWidth != -1) && (initialHeight != -1))
		{
			auto localAtlasData = atlasData();
			OnAtlasSizeChange(
				localAtlasData.width * localAtlasData.cellWidth,
				localAtlasData.height * localAtlasData.cellHeight);
		}
	}

	TextureSRV FontAtlas::atlas()
	{
		return m_atlasSRV;
	}

	void FontAtlas::OnAtlasSizeChange(int width, int height)
	{
		int newWidth = roundUpToPow2(width);
		int newHeight = roundUpToPow2(height);

		if (!m_atlasSRV ||
			newWidth != m_atlasSRV.resource().width() ||
			newHeight != m_atlasSRV.resource().height())
		{
			auto newAtlasUAV = m_device.createTextureUAV(engine::TextureDescription()
				.width(newWidth)
				.height(newHeight)
				.dimension(engine::ResourceDimension::Texture2D)
				.format(engine::Format::R8_UNORM)
				.name("Font atlas"));
			auto newAtlasSRV = m_device.createTextureSRV(newAtlasUAV);

			if (m_atlasSRV)
			{
				{
					auto cmd = m_device.createCommandList("Atlas resize copy");
					cmd.copyTexture(
						m_atlasSRV,
						newAtlasUAV,
						0, 0, 0,
						0, 0, 0,
						m_atlasSRV.resource().width(),
						m_atlasSRV.resource().height(),
						1);
					m_device.submitBlocking(cmd);
				}
				m_atlasSRV = newAtlasSRV;
				m_atlasUAV = newAtlasUAV;
			}
			else
			{
				m_atlasSRV = newAtlasSRV;
				m_atlasUAV = newAtlasUAV;
			}
		}
	}
}
