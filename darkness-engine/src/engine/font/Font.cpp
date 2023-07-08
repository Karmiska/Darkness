#include "engine/font/Font.h"
#include "engine/font/FontAtlas.h"
#include "tools/AtlasAllocator.h"
#include "tools/PathTools.h"
#include "platform/File.h"
#include "tools/Debug.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace engine
{
	Font::Font(Device& device, FT_Library library, const engine::string& fontPath)
		: m_device{ device }
		, m_face{}
		, m_atlas{ nullptr }
	{
		ASSERT(engine::fileExists(fontPath), "Tried to create font from: %s, but the file doesn't exist", fontPath.c_str());
		m_renderNodeData.resize(1024 * 1024);
		m_ringBuffer = tools::RingBuffer(
			tools::ByteRange{ 
				reinterpret_cast<uint8_t*>(&m_renderNodeData[0]), 
				reinterpret_cast<uint8_t*>(&m_renderNodeData[0]) + 
					m_renderNodeData.size()*sizeof(GlyphRenderNodeData) });
		auto error = FT_New_Face(library, fontPath.c_str(), 0, &m_face);
		// FT_New_Memory_Face
		if (error)
		{
			switch (error)
			{
				case FT_Err_Unknown_File_Format:
				{
					LOG_ERROR("FreeType error. Unknown file format");
					break;
				}
				default:
				{
					LOG_ERROR("FreeType error. Error opening file. (could be a read failure or corrupt font file)");
				}
			}
		}
		createAtlasWithFontSize(24.6f);
	}

	int Font::pointSizeFromPixel(float pixelSize)
	{
		return static_cast<int>(((pixelSize * 72.0f) / 300.0f) * 64.0f);
	}

	int Font::validGlyphCount()
	{
		int result = 0;
		for (int n = 0; n < 255; n++)
		{
			//auto glyph_index = FT_Get_Char_Index(m_face, n);

			/* load glyph image into the slot (erase previous one) */
			auto error = FT_Load_Char(m_face, n, FT_LOAD_RENDER);
			if (error)
				continue;  /* ignore errors */

			//auto error = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
			//if (error)
			//	continue;  /* ignore errors, jump to next glyph */
			//
			//FT_Glyph      glyph;
			//error = FT_Get_Glyph(m_face->glyph, &glyph);
			//if (error)
			//	continue;  /* ignore errors, jump to next glyph */
			//
			//

#if 0




			FT_GlyphSlot  slot;
			FT_Matrix     matrix;              /* transformation matrix */
			FT_UInt       glyph_index;
			FT_Vector     pen;                 /* untransformed origin */
			int           n;

			slot = m_face->glyph;                /* a small shortcut */

/* set up matrix */
			float angle = 0.0f;
			matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
			matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
			matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
			matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

			long my_target_height = fontSize;

			/* the pen position in 26.6 cartesian space coordinates */
			/* start at (300,200)                                   */
			pen.x = 0 * 64;
			pen.y = 0;// (my_target_height - 200) * 64;

			int maxWidth = 0;
			int maxHeight = 0;

			int glyphCount = 0;


			FT_Vector     pen;
			int xPos = 0;
			int yPos = 0;

			engine::vector<engine::ResidencyManagerV2::ResidencyFuture> futures;

			for (int n = 0; n < 255; n++)
			{
				/* set transformation */
				FT_Set_Transform(m_face, &matrix, &pen);

				/* load glyph image into the slot (erase previous one) */
				error = FT_Load_Char(m_face, n, FT_LOAD_RENDER);
				if (error)
					continue;  /* ignore errors */

					/* now, draw to our target surface (convert position) */
				//my_draw_bitmap(&slot->bitmap,
				//	slot->bitmap_left,
				//	my_target_height - slot->bitmap_top);
				if (slot->bitmap.buffer)
				{
					auto atlasAlloc = m_atlas->allocate();
					futures.emplace_back(m_device.residencyV2().upload(
						static_cast<void*>(slot->bitmap.buffer),
						slot->bitmap.width,
						slot->bitmap.rows,
						m_atlas->atlas().texture(),
						//xPos, yPos,
						atlasAlloc.x, atlasAlloc.y,
						0, 0, 1, true));

					GlyphStore store;
					store.x = atlasAlloc.x;
					store.y = atlasAlloc.y;
					store.width = slot->bitmap.width;
					store.height = slot->bitmap.rows;
					m_glyphs[n] = store;
				}
				/*auto cmd = device.createCommandList("Glyph upload cmd");
				cmd.copyTexture()
				device.submitBlocking(cmd);*/

				/* increment pen position */
				pen.x += slot->advance.x;
				pen.y += slot->advance.y;

			}

			for (auto&& future : futures)
				future.blockUntilUploaded();

#endif

			if (m_face->glyph->bitmap.buffer)
				++result;
		}
		return result;
	}

	Vector2<int> Font::maxGlyphSize()
	{
		Vector2<int> result{ 0, 0 };

		for (int n = 0; n < 255; n++)
		{
			auto glyph_index = FT_Get_Char_Index(m_face, n);

			FT_Glyph      glyph;

			auto error = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
			if (error)
				continue;  /* ignore errors, jump to next glyph */

			error = FT_Get_Glyph(m_face->glyph, &glyph);
			if (error)
				continue;  /* ignore errors, jump to next glyph */

			FT_BBox  glyph_bbox;
			FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &glyph_bbox);

			if (result.x < glyph_bbox.xMax - glyph_bbox.xMin)
				result.x = glyph_bbox.xMax - glyph_bbox.xMin;

			if (result.y < glyph_bbox.yMax - glyph_bbox.yMin)
				result.y = glyph_bbox.yMax - glyph_bbox.yMin;
		}

		return result;
	}

	void Font::createAtlasWithInitialSize(const engine::Vector2<int>& maxGlyphSize, int validGlyphCount)
	{
		engine::Vector2<int> initialSize;
		{
			AtlasAllocator atlas(maxGlyphSize.x, maxGlyphSize.y);
			initialSize = atlas.simulateSize(validGlyphCount);
		}

		m_atlas = engine::make_shared<FontAtlas>(
			m_device,
			maxGlyphSize.x,
			maxGlyphSize.y,
			initialSize.x,
			initialSize.y);
	}

	void Font::loadGlyphsToAtlas()
	{
		FT_Matrix     matrix;
		float angle = 0.0f;
		matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
		matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
		matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
		matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

		FT_Vector     pen{ 0, 0 };
		FT_GlyphSlot slot = m_face->glyph;

		engine::vector<engine::ResidencyManagerV2::ResidencyFuture> futures;

		for (int n = 0; n < 255; n++)
		{
			FT_Set_Transform(m_face, &matrix, &pen);

			auto error = FT_Load_Char(m_face, n, FT_LOAD_RENDER);
			if (error)
				continue;  /* ignore errors */

			//FT_Render_Glyph(slot, FT_Render_Mode::FT_RENDER_MODE_NORMAL);

			GlyphStore store;
			store.width = m_face->glyph->bitmap.width;
			store.height = m_face->glyph->bitmap.rows;
			store.xAdvance = slot->advance.x >> 6;
			store.brearingX = slot->metrics.horiBearingX;
			store.bitmapTop = slot->bitmap_top - store.height;
			if (m_face->glyph->bitmap.buffer)
			{
				auto atlasAlloc = m_atlas->allocate();
				futures.emplace_back(m_device.residencyV2().upload(
					static_cast<void*>(m_face->glyph->bitmap.buffer),
					m_face->glyph->bitmap.width,
					m_face->glyph->bitmap.rows,
					m_atlas->atlas().texture(),
					//xPos, yPos,
					atlasAlloc.x, atlasAlloc.y,
					0, 0, 1, true));

				
				store.x = atlasAlloc.x;
				store.y = atlasAlloc.y;
				store.cellHeight = atlasAlloc.height;
				store.validBitmap = true;
			}
			else
			{
				store.x = 0;
				store.y = 0;
				store.validBitmap = false;
			}
			m_glyphs[n] = store;

			//pen.x += slot->advance.x;
			//pen.y += slot->advance.y;

		}

		for (auto&& future : futures)
			future.blockUntilUploaded();
	}

	void Font::createAtlasWithFontSize(float pixelHeight)
	{
		const int fontSize = pointSizeFromPixel(pixelHeight);

		auto error = FT_Set_Char_Size(
			m_face,    /* handle to face object           */
			0,       /* char_width in 1/64th of points  */
			fontSize,   /* char_height in 1/64th of points */
			300,     /* horizontal device resolution    */
			300);   /* vertical device resolution      */
		if (error)
			LOG("FreeType complained about something. We're not that interested.");

		auto maxSize = maxGlyphSize();
		auto glyphCount = validGlyphCount();

		createAtlasWithInitialSize(maxSize, glyphCount);
		loadGlyphsToAtlas();
	}

	Font::~Font()
	{
		FT_Done_Face(m_face);
	}

	TextureSRV Font::fontAtlas()
	{
		return m_atlas->atlas();
	}

	void Font::freeTemporaryAllocations()
	{
		m_ringBuffer.reset();
	}

	Font::GlyphRenderData Font::renderText(const engine::string& text)
	{
		//FT_GlyphSlot slot = m_face->glyph;
		//
		//FT_Matrix     matrix;
		//float angle = 0.0f;
		//matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
		//matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
		//matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
		//matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
		//
		//FT_UInt       previous;
		FT_Vector     pen;
		pen.x = 0 * 64;
		pen.y = 0;
		//
		GlyphRenderData result;
		result.texture = m_atlas->atlas();

		//bool use_kerning = FT_HAS_KERNING(m_face);

		//result.nodes.reserve(text.length());
		auto allocation = m_ringBuffer.allocate(sizeof(GlyphRenderNodeData) * text.length());
		result.nodes = reinterpret_cast<GlyphRenderNodeData*>(allocation.ptr);
		result.nodeCount = static_cast<int>(text.length());
		int nodeIndex = 0;
		for (int i = 0; i < text.length(); ++i)
		{
			//auto glyph_index = FT_Get_Char_Index(m_face, text[i]);
			//if (use_kerning && previous && glyph_index)
			//{
			//	FT_Vector  delta;
			//
			//
			//	FT_Get_Kerning(m_face, previous, glyph_index,
			//		FT_KERNING_DEFAULT, &delta);
			//
			//	pen.x += delta.x >> 6;
			//}
			//
			//FT_Set_Transform(m_face, &matrix, &pen);
			//
			//auto error = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER);
			//if (error) continue;
			//
			

			GlyphRenderNodeData& node = result.nodes[nodeIndex];
			GlyphStore& store = m_glyphs[text[i]];
			if (store.validBitmap)
			{
				node.uvTopLeft = {
					static_cast<float>(store.x) / static_cast<float>(result.texture.width()),
					static_cast<float>(store.y) / static_cast<float>(result.texture.height()) };
				node.uvBottomRight = {
					static_cast<float>(store.x + store.width) / static_cast<float>(result.texture.width()),
					static_cast<float>(store.y + store.height) / static_cast<float>(result.texture.height()) };
				node.size = { static_cast<float>(store.width), static_cast<float>(store.height) };
				node.dstPosition = { static_cast<float>(pen.x + (store.brearingX >> 6)), static_cast<float>(pen.y + (store.cellHeight - store.height) - store.bitmapTop) };
			}
			pen.x += store.xAdvance;
			++nodeIndex;
			//pen.x += slot->advance.x >> 6;
			//pen.y += slot->advance.y;
			//
			//previous = glyph_index;
		}

		return result;
	}

}
