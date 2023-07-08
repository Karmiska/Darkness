#pragma once

#include "containers/vector.h"
#include "containers/memory.h"

namespace engine
{
	namespace image
	{
		class ImageIf;
	}
	class Device;
}

namespace ui
{
	enum GridImages
	{
		TopLeft = 0x1,
		TopCenter = 0x2,
		TopRight = 0x4,
		MiddleLeft = 0x8,
		MiddleCenter = 0x10,
		MiddleRight = 0x20,
		BottomLeft = 0x40,
		BottomCenter = 0x80,
		BottomRight = 0x100
	};

	class DrawCommandBuffer;

	class GridImage
	{
	public:
		GridImage(engine::Device& device, const char* imageName);

		void draw(
			DrawCommandBuffer& cmd,
			int x, int y, 
			int width, int height, 
			unsigned int images);
	private:
		engine::vector<engine::shared_ptr<engine::image::ImageIf>> m_images;
	};
}
