#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "tools/image/ImageIf.h"
#include "engine/graphics/Format.h"
#include "BmpBoilerplate.h"

namespace engine
{
    namespace image
    {
        class Bmp : public ImageIf
        {
        public:
            Bmp(const engine::string& filename);
            Bmp(const engine::string& filename,
                Format format,
                unsigned int width,
                unsigned int height,
                unsigned int slices,
                unsigned int mips);

            size_t width() const override;
            size_t height() const override;
            Format format() const override;
            size_t mipCount() const override;
            size_t arraySlices() const override;

            ImageSubresource map(
                size_t mipLevel,
                size_t arraySlice) const override;

            void reserve() override;
            void save(const char* data, size_t bytes) override;
            void save() override;
            const uint8_t* data() const override;
            size_t bytes() const override;

            void flipVertical() override;
            void convert() override;
        private:
            engine::string mFilename;
            BitmapFileHeader mFileHeader;
            BimapV3Header mDibHeader;
            Format m_format;
            engine::vector<uint8_t> m_data;

            void readHeader(std::fstream& file);
            void createBmpHeader(
                Format format,
                unsigned int width,
                unsigned int height);

            void readPixels(std::fstream& file);
            void writePixels(std::fstream& file);
            inline void convert(const uint8_t* src, uint8_t* dst, uint32_t bytes) const;
            inline void convertFrom(const uint8_t* src, uint8_t* dst, uint32_t bytes) const;
        };
    }
}
