#include "ImageHelper.h"

#include "containers/memory.h"
#include "containers/string.h"
#include <fstream>

#include "tools/Debug.h"
#include "tools/ToolsCommon.h"

#define  FREEIMAGE_LIB
#include "FreeImage.h"
#include "Utilities.h"

void freeImageOutput(FREE_IMAGE_FORMAT /*fif*/, const char *msg)
{
    LOG("%s", msg);
}

namespace resource_task
{
    internals::FreeImageInitializer initFreeImage;

    namespace internals
    {
        FreeImageInitializer::FreeImageInitializer()
        {
            FreeImage_Initialise();
            FreeImage_SetOutputMessage(freeImageOutput);
        }

        FreeImageInitializer::~FreeImageInitializer()
        {
            FreeImage_DeInitialise();
        }

        ChannelHelper::ChannelHelper(
            uint32_t imageType,
            uint32_t bpp)
            : m_channelFormat{ channelFormatFromType(imageType, bpp) }
            , m_channelBits{ channelBitsFromFormat(m_channelFormat) }
            , m_channels{ channelsFromType(imageType, bpp) }
        {
        }

        uint32_t ChannelHelper::channels() const
        {
            return m_channels;
        }

        uint32_t ChannelHelper::channelBits() const
        {
            return m_channelBits;
        }

        ChannelFormat ChannelHelper::channelFormat() const
        {
            return m_channelFormat;
        }

        ChannelFormat ChannelHelper::channelFormatFromType(
            uint32_t imageType,
            uint32_t bpp)
        {
            FREE_IMAGE_TYPE type = static_cast<FREE_IMAGE_TYPE>(imageType);
            switch (type)
            {
                case FIT_UNKNOWN: return ChannelFormat::Unknown;
                case FIT_BITMAP:
                {
                    //! standard image			: 1-, 4-, 8-, 16-, 24-, 32-bit
                    switch (bpp)
                    {
                    case 8: return ChannelFormat::Uint8;
                    case 16: return ChannelFormat::Uint8;
                    case 24: return ChannelFormat::Uint8;
                    case 32: return ChannelFormat::Uint8;
                    default: return ChannelFormat::Uint8;
                    }
                }
                case FIT_UINT16: return ChannelFormat::Uint16;
                case FIT_INT16: return ChannelFormat::Int16;
                case FIT_UINT32: return ChannelFormat::Uint32;
                case FIT_INT32: return ChannelFormat::Int32;
                case FIT_FLOAT: return ChannelFormat::Float32;
                case FIT_DOUBLE: return ChannelFormat::Float64;
                case FIT_COMPLEX: return ChannelFormat::Float64;
                case FIT_RGB16: return ChannelFormat::Uint16;	//! 48-bit RGB image			: 3 x 16-bit
                case FIT_RGBA16: return ChannelFormat::Uint16;	//! 64-bit RGBA image		: 4 x 16-bit
                case FIT_RGBF: return ChannelFormat::Float32;
                case FIT_RGBAF: return ChannelFormat::Float32;
            }
            return ChannelFormat::Unknown;
        }

        uint32_t ChannelHelper::channelBitsFromFormat(ChannelFormat format)
        {
            switch (format)
            {
                case ChannelFormat::Unknown: return 0;
                case ChannelFormat::Uint8: return 8;
                case ChannelFormat::Uint16: return 16;
                case ChannelFormat::Uint32: return 32;
                case ChannelFormat::Int16: return 16;
                case ChannelFormat::Int32: return 32;
                case ChannelFormat::Float16: return 16;
                case ChannelFormat::Float32: return 32;
                case ChannelFormat::Float64: return 64;
            }
            return 0;
        }

        uint32_t ChannelHelper::channelsFromType(
            uint32_t imageType,
            uint32_t bpp)
        {
            FREE_IMAGE_TYPE type = static_cast<FREE_IMAGE_TYPE>(imageType);
            switch (type)
            {
                case FIT_UNKNOWN: return 0;
                case FIT_BITMAP:
                {
                    //! standard image			: 1-, 4-, 8-, 16-, 24-, 32-bit
                    if (bpp < 8)
                        return 0;

                    return bpp / 8;
                }
                case FIT_UINT16: return 1;
                case FIT_INT16: return 1;
                case FIT_UINT32: return 1;
                case FIT_INT32: return 1;
                case FIT_FLOAT: return 1;
                case FIT_DOUBLE: return 1;
                case FIT_COMPLEX: return 2;
                case FIT_RGB16: return 3;
                case FIT_RGBA16: return 4;
                case FIT_RGBF: return 3;
                case FIT_RGBAF: return 4;
            }
            return 0;
        }
    }

    ImageData::ImageData(const engine::string& file)
        : m_imageMemory{ nullptr }
        , m_bitmap{ nullptr }
        , m_data{ nullptr }
        , m_bytes{ 0u }
        , m_stride{ 0u }
        , m_width{ 0u }
        , m_height{ 0u }
        , m_ownsBitmap{ false }
    {
        engine::vector<char> srcBuffer;

        std::ifstream src;
        src.open(file.c_str(), std::ios::binary | std::ios::in);
        src.seekg(0, std::ios::end);
        srcBuffer.resize(src.tellg());
        src.seekg(0, std::ios::beg);
        src.read(&srcBuffer[0], srcBuffer.size());
        src.close();

        m_imageMemory = FreeImage_OpenMemory(
            reinterpret_cast<BYTE*>(srcBuffer.data()),
            static_cast<DWORD>(srcBuffer.size()));

        FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileTypeFromMemory(static_cast<FIMEMORY*>(m_imageMemory));
        m_bitmap = FreeImage_LoadFromMemory(imageFormat, static_cast<FIMEMORY*>(m_imageMemory), 0);
        m_ownsBitmap = true;
        
        m_data = FreeImage_GetBits(static_cast<FIBITMAP*>(m_bitmap));
        m_stride = FreeImage_GetPitch(static_cast<FIBITMAP*>(m_bitmap));
        m_width = FreeImage_GetWidth(static_cast<FIBITMAP*>(m_bitmap));
        m_height = FreeImage_GetHeight(static_cast<FIBITMAP*>(m_bitmap));
        m_bytes = m_stride * m_height;
    }

    ImageData::ImageData(const ImageData& data, const ImageSize& size)
        : m_imageMemory{ nullptr }
        , m_bitmap{ nullptr }
        , m_data{ nullptr }
        , m_bytes{ 0u }
        , m_stride{ 0u }
        , m_width{ 0u }
        , m_height{ 0u }
        , m_ownsBitmap{ false }
    {
        if (size.width != data.width() || size.height != data.height())
        {
            m_bitmap = FreeImage_Rescale(
                static_cast<FIBITMAP*>(data.bitmap()),
                size.width,
                size.height,
                FREE_IMAGE_FILTER::FILTER_LANCZOS3);
            m_ownsBitmap = true;

            m_data = FreeImage_GetBits(static_cast<FIBITMAP*>(m_bitmap));
            m_stride = FreeImage_GetPitch(static_cast<FIBITMAP*>(m_bitmap));
            m_width = FreeImage_GetWidth(static_cast<FIBITMAP*>(m_bitmap));
            m_height = FreeImage_GetHeight(static_cast<FIBITMAP*>(m_bitmap));
            m_bytes = m_stride * m_height;
        }
        else
        {
            m_bitmap = data.bitmap();
            m_ownsBitmap = false;

            m_data = FreeImage_GetBits(static_cast<FIBITMAP*>(m_bitmap));
            m_stride = FreeImage_GetPitch(static_cast<FIBITMAP*>(m_bitmap));
            m_width = FreeImage_GetWidth(static_cast<FIBITMAP*>(m_bitmap));
            m_height = FreeImage_GetHeight(static_cast<FIBITMAP*>(m_bitmap));
            m_bytes = m_stride * m_height;
        }
    }

    ImageData::~ImageData()
    {
        if(m_ownsBitmap && m_bitmap)
            FreeImage_Unload(static_cast<FIBITMAP*>(m_bitmap));
        if(m_imageMemory)
            FreeImage_CloseMemory(static_cast<FIMEMORY*>(m_imageMemory));
    }

    void* ImageData::bitmap() const
    {
        return m_bitmap;
    }

    void* ImageData::data() const
    {
        return m_data;
    }

    size_t ImageData::bytes() const
    {
        return m_bytes;
    }

    uint32_t ImageData::stride() const
    {
        return m_stride;
    }

    uint32_t ImageData::width() const
    {
        return m_width;
    }

    uint32_t ImageData::height() const
    {
        return m_height;
    }

    ImageHelper::ImageHelper(const ImageData& image)
    {
        FIBITMAP* bitmap = static_cast<FIBITMAP*>(image.bitmap());

        m_width = FreeImage_GetWidth(bitmap);
        m_height = FreeImage_GetHeight(bitmap);

		auto imageType = FreeImage_GetImageType(bitmap);
        internals::ChannelHelper channelHelper(
            imageType, 
            FreeImage_GetBPP(bitmap));
        
        m_channels = channelHelper.channels();
        m_channelFormat = channelHelper.channelFormat();

        unsigned int redMask = FreeImage_GetRedMask(bitmap);
        unsigned int greenMask = FreeImage_GetGreenMask(bitmap);
        unsigned int blueMask = FreeImage_GetBlueMask(bitmap);
        bool transparent = FreeImage_IsTransparent(bitmap);

		if (redMask == 0 && greenMask == 0 && blueMask == 0 && imageType == FIT_RGBA16)
		{
			redMask = 0xff << 24;
			greenMask = 0xff << 16;
			blueMask = 0xff << 8;
			m_channelOrder = ChannelOrder::RGBA;
		}

#if 0
        if (!transparent)
        {
            if (redMask > greenMask && redMask > blueMask)
                m_channelOrder = ChannelOrder::RGB;
            else if (blueMask > greenMask && blueMask > redMask)
                m_channelOrder = ChannelOrder::BGR;
            else
                ASSERT(false, "No support for channel orders where green comes first");
        }
        else
        {
            bool alphaZero = redMask > 0xff && greenMask > 0xff && blueMask > 0xff;

            if (alphaZero && redMask > greenMask && redMask > blueMask)
                m_channelOrder = ChannelOrder::RGBA;
            else if (alphaZero && blueMask > greenMask && blueMask > redMask)
                m_channelOrder = ChannelOrder::BGRA;
            else if (!alphaZero && redMask > greenMask && redMask > blueMask)
                m_channelOrder = ChannelOrder::ARGB;
            else if (!alphaZero && blueMask > greenMask && blueMask > redMask)
                m_channelOrder = ChannelOrder::ABGR;
            else
                ASSERT(false, "No support for channel orders where green comes first");
        }
#else
        // FreeImage stores stuff in BGR format on little endian devices
        if (m_channelFormat == ChannelFormat::Float32 && m_channels == 3)
        {
            // hdr image?
            m_channelOrder = ChannelOrder::RGB;
        }
        else if (m_channelFormat == ChannelFormat::Float32 && m_channels == 1)
        {
            // hdr image?
            m_channelOrder = ChannelOrder::R;
        }
        else if (m_channelFormat == ChannelFormat::Float32 && m_channels == 2)
        {
            // hdr image?
            m_channelOrder = ChannelOrder::RG;
        }
        else if (!transparent && m_channels == 3)
        {
            if (redMask > greenMask && redMask > blueMask)
                m_channelOrder = ChannelOrder::BGR;
            else if (blueMask > greenMask && blueMask > redMask)
                m_channelOrder = ChannelOrder::BGR;
            else
                ASSERT(false, "No support for channel orders where green comes first");
        }
        else if(m_channels == 4)
        {
            bool alphaZero = redMask > 0xff && greenMask > 0xff && blueMask > 0xff;

            if (alphaZero && redMask > greenMask && redMask > blueMask && greenMask > blueMask)
                m_channelOrder = ChannelOrder::RGBA;
            else if (alphaZero && blueMask > greenMask && blueMask > redMask)
                m_channelOrder = ChannelOrder::BGRA;
            else if (!alphaZero && redMask > greenMask && redMask > blueMask)
                m_channelOrder = ChannelOrder::ABGR;
            else if (!alphaZero && blueMask > greenMask && blueMask > redMask)
                m_channelOrder = ChannelOrder::ABGR;
            else
                ASSERT(false, "No support for channel orders where green comes first");
        }
		else if (m_channels == 2)
		{
			m_channelOrder = ChannelOrder::RG;
		}
		else if (m_channels == 1)
		{
			m_channelOrder = ChannelOrder::R;
		}
#endif
    }

    uint32_t ImageHelper::width() const
    {
        return m_width;
    }

    uint32_t ImageHelper::height() const
    {
        return m_height;
    }

    uint32_t ImageHelper::channels() const
    {
        return m_channels;
    }

    ChannelFormat ImageHelper::channelFormat() const
    {
        return m_channelFormat;
    }

    ChannelOrder ImageHelper::channelOrder() const
    {
        return m_channelOrder;
    }

    ImageSize makeGoodSize(
        const ImageSize& size, 
        uint32_t maximumSize, 
        bool /*hasToBePowerOfTwo*/)
    {
        ImageSize result = size;

        if (result.width > maximumSize) result.width = maximumSize;
        if (result.height > maximumSize) result.height = maximumSize;

        if (result.width % 4 != 0 || !isPowerOfTwo(result.width))
        {
            int potSize = 1;
            while (static_cast<unsigned int>(potSize) < result.width) potSize *= 2;
            result.width = potSize;
        }

        if (result.height % 4 != 0 || !isPowerOfTwo(result.height))
        {
            int potSize = 1;
            while (static_cast<unsigned int>(potSize) < result.height) potSize *= 2;
            result.height = potSize;
        }

        return result;
    }

    engine::vector<ImageSize> mipInfo(
        const ImageSize& size, 
        bool blockCompression)
    {
        engine::vector<ImageSize> result;

        if (!blockCompression)
        {
            auto w = size.width;
            auto h = size.height;
            result.emplace_back(ImageSize{ w, h });

            while (w > 1 || h > 1)
            {
                w /= 2;
                h /= 2;

                if (w < 1) w = 1;
                if (h < 1) h = 1;

                result.emplace_back(ImageSize{ w, h });
            }
        }
        else
        {
            auto w = size.width;
            auto h = size.height;
            w = roundUpToMultiple(w, 4);
            h = roundUpToMultiple(h, 4);

            result.emplace_back(ImageSize{ w, h });

            while (w > 4 || h > 4)
            {
                w /= 2;
                h /= 2;

                w = roundUpToMultiple(w, 4);
                h = roundUpToMultiple(h, 4);

                if (w < 4) w = 4;
                if (h < 4) h = 4;

                result.emplace_back(ImageSize{ w, h });
            }
        }

        return result;
    }
}