#pragma once

#include "containers/string.h"
#include <stdint.h>
#include "containers/vector.h"

namespace resource_task
{
    enum class ChannelFormat
    {
        Unknown,

        Uint8,
        Uint16,
        Uint32,

        Int16,
        Int32,

        Float16,
        Float32,
        Float64
    };

    enum class ChannelOrder
    {
		R,
		RG,
        BGR,
        RGB,
        BGRA,
        RGBA,
        ABGR,
        ARGB
    };

    namespace internals
    {
        class FreeImageInitializer
        {
        public:
            FreeImageInitializer();
            ~FreeImageInitializer();
        };
        extern FreeImageInitializer initFreeImage;

        class ChannelHelper
        {
        public:
            ChannelHelper(
                uint32_t imageType,
                uint32_t bpp);

            uint32_t channels() const;
            uint32_t channelBits() const;
            ChannelFormat channelFormat() const;
        private:
            ChannelFormat m_channelFormat;
            uint32_t m_channelBits;
            uint32_t m_channels;
            
            ChannelFormat channelFormatFromType(
                uint32_t imageType,
                uint32_t bpp);
            uint32_t channelBitsFromFormat(ChannelFormat format);
            uint32_t channelsFromType(
                uint32_t imageType,
                uint32_t bpp);
        };
    }
    
    struct ImageSize
    {
        uint32_t width;
        uint32_t height;
    };

    class ImageData
    {
    public:
        ImageData(const engine::string& file);
        ImageData(const ImageData& data, const ImageSize& size);
        ~ImageData();

        void* bitmap() const;
        void* data() const;
        size_t bytes() const;
        uint32_t stride() const;
        uint32_t width() const;
        uint32_t height() const;
    private:
        void* m_imageMemory;
        void* m_bitmap;
        void* m_data;
        uint32_t m_bytes;
        uint32_t m_stride;
        uint32_t m_width;
        uint32_t m_height;
        bool m_ownsBitmap;
    };

    class ImageHelper
    {
    public:
        ImageHelper(const ImageData& image);

        uint32_t width() const;
        uint32_t height() const;
        uint32_t channels() const;
        ChannelFormat channelFormat() const;
        ChannelOrder channelOrder() const;
    private:
        
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_channels;
        ChannelFormat m_channelFormat;
        ChannelOrder m_channelOrder;
    };
    
    ImageSize makeGoodSize(
        const ImageSize& size, 
        uint32_t maximumSize, 
        bool hasToBePowerOfTwo = true);

    engine::vector<ImageSize> mipInfo(
        const ImageSize& size, 
        bool blockCompression = false);
}
