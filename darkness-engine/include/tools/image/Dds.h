#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "engine/graphics/Format.h"
#include "tools/image/ImageIf.h"
#include "engine/primitives/Color.h"
#include "DdsBoilerplate.h"

namespace engine
{
    namespace image
    {
        class Dds : public ImageIf
        {
        public:
            Dds(const engine::string& filename);
            Dds(const engine::string& filename,
                Format type,
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
            bool readHeader(std::fstream& file);
            void createHeader(unsigned int width, unsigned int height, Format format);
            void loadImage(std::fstream& file);
            size_t slicePitch() const;

            engine::vector<uint8_t> m_data;
            engine::string m_filename;
            DdsHeader m_header;
            DdsHeaderDXT10 m_extendedHeader;
            Format m_format;

            size_t m_slices;
            size_t m_mipmaps;
            size_t m_fileSize;
        };

    }
}
