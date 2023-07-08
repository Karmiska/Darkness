#include "tools/image/Bmp.h"
#include "tools/image/Color.h"
#include <fstream>
#include <stdio.h>
#include <math.h>

using namespace engine;
using namespace bmp;

#define HeaderFieldValue(sig) (((int)sig[0]) | ((int)sig[1] << 8))

namespace engine
{
    namespace image
    {
        Bmp::Bmp(const engine::string& filename)
            : mFilename(filename)
            , m_format{ Format::R8G8B8A8_UINT }
        {
            std::fstream input;
            input.open(filename.c_str(), std::ios::binary | std::ios::in);
            if (input.is_open())
            {
                readHeader(input);
                readPixels(input);
                input.close();
            }
        }

        void Bmp::readHeader(std::fstream& file)
        {
            // read bmp headers
            memset(&mFileHeader, 0, sizeof(mFileHeader));
            memset(&mDibHeader, 0, sizeof(mDibHeader));
            file.read(reinterpret_cast<char*>(&mFileHeader), sizeof(BitmapFileHeader));
            file.read(reinterpret_cast<char*>(&mDibHeader), sizeof(BimapV3Header));
            switch (mDibHeader.bitsPerPixel)
            {
            case 16:
            {
                m_format = Format::B5G6R5_UNORM;
                break;
            }
            case 24:
            {
                m_format = Format::R8G8B8A8_UINT;
                break;
            }
            case 32:
            {
                m_format = Format::R8G8B8A8_UINT;
                break;
            }
            }
        }

        Bmp::Bmp(const engine::string& filename,
            Format format,
            unsigned int width,
            unsigned int height,
            unsigned int /*slices*/,
            unsigned int /*mips*/)
            : mFilename(filename)
            , m_format{ format }
        {
            createBmpHeader(format, width, height);
        }

        void Bmp::createBmpHeader(
            Format format,
            unsigned int width,
            unsigned int height)
        {
            memset(&mFileHeader, 0, sizeof(mFileHeader));
            memset(&mDibHeader, 0, sizeof(mDibHeader));

            mFileHeader.signature = HeaderFieldValue("BM");

            mDibHeader.size = sizeof(BimapV3Header);
            mDibHeader.width = width;
            mDibHeader.height = height;
            mDibHeader.planes = 1;
            mDibHeader.bitsPerPixel = static_cast<unsigned short>(formatBits(format));
            mDibHeader.compression = 0;

            // setup
            int bytesPerLine = static_cast<int>(formatBytes(format, width, 1));

            mDibHeader.imageSize = bytesPerLine * height;
            mDibHeader.xPixelsPerMeter = DEFAULT_PPM_X;
            mDibHeader.yPixelsPerMeter = DEFAULT_PPM_Y;

            mFileHeader.offsetToPixelArray = sizeof(BitmapFileHeader) + mDibHeader.size;
            mFileHeader.size = mFileHeader.offsetToPixelArray + mDibHeader.imageSize;
        }

        void Bmp::readPixels(std::fstream& file)
        {
            m_data.resize(formatBytes(m_format, static_cast<unsigned int>(width()), static_cast<unsigned int>(height())));
            file.seekg(mFileHeader.offsetToPixelArray, std::ios::beg);
            auto ptr = map(0, 0);
            file.read(reinterpret_cast<char*>(const_cast<uint8_t*>(ptr.data)), ptr.sizeBytes);
        }

        size_t Bmp::width() const
        {
            return static_cast<size_t>(mDibHeader.width);
        }

        size_t Bmp::height() const
        {
            return static_cast<size_t>(mDibHeader.height);
        }

        Format Bmp::format() const
        {
            return m_format;
        }

        size_t Bmp::mipCount() const
        {
            return 1;
        }

        size_t Bmp::arraySlices() const
        {
            return 1;
        }

        ImageSubresource Bmp::map(
            size_t mipLevel,
            size_t arraySlice) const
        {
            ASSERT(mipLevel == 0, "BMP codec does not currently support other than mip level 0");
            ASSERT(arraySlice == 0, "BMP codec does not currently support other than slice 0");

            ImageSubresource res;
            res.width = width();
            res.height = height();
            res.data = m_data.data();
            res.pitch = formatBytes(m_format, mDibHeader.width, 1);
            res.slicePitch = res.pitch * mDibHeader.height;
            res.sizeBytes = res.slicePitch;
            return res;
        }

        void Bmp::save()
        {
            std::fstream input;
            input.open(mFilename.c_str(), std::ios::binary | std::ios::out);
            if (input.is_open())
            {
                // headers have been already setup in constructors
                input.write(reinterpret_cast<char*>(&mFileHeader), sizeof(BitmapFileHeader));
                input.write(reinterpret_cast<char*>(&mDibHeader), sizeof(BimapV3Header));
                writePixels(input);
                input.close();
            }
        }

        void Bmp::reserve()
        {
            m_data.resize(formatBytes(m_format, static_cast<unsigned int>(width()), static_cast<unsigned int>(height())));
        }

        void Bmp::save(const char* data, size_t bytes)
        {
            m_data.resize(bytes);
            memcpy(&m_data[0], data, bytes);
            save();
        }

        const uint8_t* Bmp::data() const
        {
            return m_data.data();
        }

        size_t Bmp::bytes() const
        {
            return m_data.size();
        }

        void Bmp::writePixels(std::fstream& file)
        {
            file.write(reinterpret_cast<const char*>(m_data.data()), m_data.size());
            /*else
            {
                auto src = &m_data[0];
                src += formatBytes(m_format, width(), height());
                auto rowBytes = formatBytes(m_format, width(), 1);
                auto revPtr = src - rowBytes;
                engine::vector<uint8_t> scratchBuffer(rowBytes);
                for (int y = 0; y < height(); ++y)
                {
                    convert(revPtr, &scratchBuffer[0], rowBytes);
                    file.write(reinterpret_cast<const char*>(scratchBuffer.data()), rowBytes);
                    revPtr -= rowBytes;
                }
            }*/
        }

        void Bmp::flipVertical()
        {
            auto src = &m_data[0];
            auto dst = src + formatBytes(m_format, static_cast<unsigned int>(width()), static_cast<unsigned int>(height()));

            auto rowBytes = formatBytes(m_format, static_cast<unsigned int>(width()), 1u);

            auto revPtr = dst - rowBytes;

            engine::vector<uint8_t> scratchBuffer(rowBytes);

            for (int y = 0; y < height() / 2; ++y)
            {
                memcpy(&scratchBuffer[0], src, rowBytes);
                memcpy(src, revPtr, rowBytes);
                memcpy(revPtr, &scratchBuffer[0], rowBytes);
                
                revPtr -= rowBytes;
                src += rowBytes;
            }
        }

        void Bmp::convert()
        {
            auto src = &m_data[0];
            auto rowBytes = formatBytes(m_format, static_cast<unsigned int>(width()), 1u);
            engine::vector<uint8_t> scratchBuffer(rowBytes);
            for (int y = 0; y < height(); ++y)
            {
                convert(src, src, static_cast<uint32_t>(rowBytes));
                src += rowBytes;
            }
        }

        void Bmp::convert(const uint8_t* src, uint8_t* dst, uint32_t bytes) const
        {
            auto elements = bytes / formatBytes(m_format, 1, 1);

            const uint32_t* from = reinterpret_cast<const uint32_t*>(src);
            uint32_t* to = reinterpret_cast<uint32_t*>(dst);

            for (int i = 0; i < elements; ++i)
            {
                Color a(*from);
                Color b(a.red(), a.alpha(), a.blue(), a.green());

                *to = b.getRGBA();

                ++from;
                ++to;
            }
        }

        void Bmp::convertFrom(const uint8_t* src, uint8_t* dst, uint32_t bytes) const
        {
            auto elements = bytes / formatBytes(m_format, 1, 1);

            const uint32_t* from = reinterpret_cast<const uint32_t*>(src);
            uint32_t* to = reinterpret_cast<uint32_t*>(dst);

            for (int i = 0; i < elements; ++i)
            {
                Color a(*from);
                Color b(a.alpha(), a.red(), a.green(), a.blue());

                *to = b.getRGBA();

                ++from;
                ++to;
            }
        }
    }
}
