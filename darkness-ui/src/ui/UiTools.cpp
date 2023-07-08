#include "ui/UiTools.h"
#include "tools/image/ImageIf.h"
#include "engine/graphics/Device.h"
#include "containers/string.h"
#include "ui/Theme.h"
#include "ui/DrawCommandBuffer.h"

namespace ui
{
    GridImage::GridImage(engine::Device& device, const char* imageName)
    {
        m_images.resize(9);
        engine::string imageNameStr = imageName;

        m_images[0] = device.createImage(
            Theme::instance().image(imageNameStr + "_topLeft"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[1] = device.createImage(
            Theme::instance().image(imageNameStr + "_topCenter"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[2] = device.createImage(
            Theme::instance().image(imageNameStr + "_topRight"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[3] = device.createImage(
            Theme::instance().image(imageNameStr + "_middleLeft"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[4] = device.createImage(
            Theme::instance().image(imageNameStr + "_middleCenter"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[5] = device.createImage(
            Theme::instance().image(imageNameStr + "_middleRight"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[6] = device.createImage(
            Theme::instance().image(imageNameStr + "_bottomLeft"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[7] = device.createImage(
            Theme::instance().image(imageNameStr + "_bottomCenter"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);

        m_images[8] = device.createImage(
            Theme::instance().image(imageNameStr + "_bottomRight"),
            engine::Format::UNKNOWN,
            -1, -1, -1, -1, engine::image::ImageType::EXTERNAL);
    }

    void GridImage::draw(
		DrawCommandBuffer& cmd,
		int x, int y,
		int width, int height,
		unsigned int images)
    {
		auto bitIsSet = [](unsigned int data, unsigned int bit)->bool
		{
			return (data & bit) == bit;
		};

		bool hasTopLeft = bitIsSet(images, GridImages::TopLeft);
		bool hasTopCenter = bitIsSet(images, GridImages::TopCenter);
		bool hasTopRight = bitIsSet(images, GridImages::TopRight);

		bool hasMiddleLeft = bitIsSet(images, GridImages::MiddleLeft);
		bool hasMiddleCenter = bitIsSet(images, GridImages::MiddleCenter);
		bool hasMiddleRight = bitIsSet(images, GridImages::MiddleRight);

		bool hasBottomLeft = bitIsSet(images, GridImages::BottomLeft);
		bool hasBottomCenter = bitIsSet(images, GridImages::BottomCenter);
		bool hasBottomRight = bitIsSet(images, GridImages::BottomRight);


		auto topLeft = m_images[0];
		auto topCenter = m_images[1];
		auto topRight = m_images[2];

		auto middleLeft = m_images[3];
		auto middleCenter = m_images[4];
		auto middleRight = m_images[5];

		auto bottomLeft = m_images[6];
		auto bottomCenter = m_images[7];
		auto bottomRight = m_images[8];

		if (hasTopLeft)
		{
			cmd.drawImage(x, y, topLeft->width(), topLeft->height(), topLeft);
		}

		if (hasTopCenter)
		{
			int tx = x;
			int twidth = width;
			if (hasTopLeft)
			{
				tx += topLeft->width();
				twidth -= topLeft->width();
			}
			if (hasTopRight)
			{
				twidth -= topRight->width();
			}
			cmd.drawImage(
				tx,
				y,
				twidth,
				topCenter->height(), topCenter);
		}
		if (hasTopRight)
		{
			cmd.drawImage(x + width - topRight->width(), y, topRight->width(), topRight->height(), topRight);
		}

		if (hasMiddleLeft)
		{
			int ty = y;
			int theight = height;
			if (hasTopLeft)
			{
				ty += topLeft->height();
				theight -= topLeft->height();
			}
			if (hasBottomLeft)
				theight -= bottomLeft->height();

			if (theight > 0)
				cmd.drawImage(x, ty, middleLeft->width(), theight, middleLeft);
		}
		if (hasMiddleCenter)
		{
			int tx = x;
			int ty = y;
			int twidth = width;
			int theight = height;
			if (hasMiddleLeft)
			{
				tx += middleLeft->width();
				twidth -= middleLeft->width();
			}
			if (hasMiddleRight)
			{
				twidth -= middleRight->width();
			}
			if (hasTopCenter)
			{
				ty += topCenter->height();
				theight -= topCenter->height();
			}
			if (hasBottomCenter)
				theight -= bottomCenter->height();

			if (theight > 0)
				cmd.drawImage(tx, ty, twidth, theight, middleCenter);
		}
		if (hasMiddleRight)
		{
			int ty = y;
			int theight = height;
			if (hasTopRight)
			{
				ty += topRight->height();
				theight -= topRight->height();
			}
			if (hasBottomRight)
				theight -= bottomRight->height();

			if (theight > 0)
				cmd.drawImage(x + width - middleRight->width(), ty, middleRight->width(), theight, middleRight);
		}

		if (hasBottomLeft)
			cmd.drawImage(x, y + height - bottomLeft->height(), bottomLeft->width(), bottomLeft->height(), bottomLeft);
		if (hasBottomCenter)
		{
			int tx = x;
			int twidth = width;
			if (hasBottomLeft)
			{
				tx += bottomLeft->width();
				twidth -= bottomLeft->width();
			}
			if (hasBottomRight)
				twidth -= bottomRight->width();

			cmd.drawImage(
				tx,
				y + height - bottomCenter->height(),
				twidth,
				bottomCenter->height(), bottomCenter);
		}
		if (hasBottomRight)
		{
			int tx = x + width;
			if (hasMiddleRight)
				tx -= middleRight->width();

			cmd.drawImage(tx, y + height - bottomRight->height(), bottomRight->width(), bottomRight->height(), bottomRight);
		}
    }
}
