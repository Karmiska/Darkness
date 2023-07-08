#pragma once

#include "engine/primitives/Color.h"
#include "engine/graphics/Format.h"

namespace engine
{
    namespace image
    {
        struct ImageSubresource
        {
            const uint8_t* data;
            size_t sizeBytes;
            size_t pitch;
            size_t slicePitch;
            size_t width;
            size_t height;
        };

        class ImageIf
        {
        public:
            virtual ~ImageIf() {};
            virtual size_t width() const = 0;
            virtual size_t height() const = 0;
            virtual Format format() const = 0;
            virtual size_t mipCount() const = 0;
            virtual size_t arraySlices() const = 0;

            virtual ImageSubresource map(
                size_t mipLevel,
                size_t arraySlice) const = 0;

            virtual void reserve() = 0;
            virtual void save(const char* data, size_t bytes) = 0;
            virtual void save() = 0;
            virtual const uint8_t* data() const = 0;
            virtual size_t bytes() const = 0;

            virtual void flipVertical() = 0;
            virtual void convert() = 0;
        };

    }
}
