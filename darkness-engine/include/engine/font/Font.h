#pragma once

#include "containers/string.h"
#include "containers/unordered_map.h"
#include "engine/primitives/Vector2.h"
#include "engine/graphics/Resources.h"
#include "tools/RingBuffer.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace engine
{
	class FontAtlas;
	class Device;

	class Font
	{
	public:
		Font(Device& device, FT_Library library, const engine::string& fontPath);

		void freeTemporaryAllocations();
	private:
		int pointSizeFromPixel(float pixelSize);
		int validGlyphCount();
		Vector2<int> maxGlyphSize();
		void createAtlasWithInitialSize(const engine::Vector2<int>& maxGlyphSize, int validGlyphCount);
		void loadGlyphsToAtlas();
		void createAtlasWithFontSize(float pixelHeight);

	public:
		~Font();

		TextureSRV fontAtlas();

	public:
		struct GlyphRenderNodeData
		{
			Vector2f uvTopLeft;
			Vector2f uvBottomRight;
			Vector2f size;
			Vector2f dstPosition;
		};
		
		struct GlyphRenderData
		{
			TextureSRV texture;
			GlyphRenderNodeData* nodes;
			int nodeCount;
		};

		GlyphRenderData renderText(const engine::string& text);

	private:
		Device& m_device;
		FT_Face m_face;
		engine::shared_ptr<FontAtlas> m_atlas;

		struct GlyphStore
		{
			int x;
			int y;
			int width;
			int height;
			int cellHeight;
			int xAdvance;
			int brearingX;
			int bitmapTop;
			bool validBitmap;
		};
		engine::unordered_map<unsigned long, GlyphStore> m_glyphs;
		engine::vector<GlyphRenderNodeData> m_renderNodeData;
		tools::RingBuffer m_ringBuffer;
		engine::vector<tools::RingBuffer::AllocStruct> m_allocations;
	};
}
