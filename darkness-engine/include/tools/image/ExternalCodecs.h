#pragma once

#include "containers/string.h"
#include "tools/image/ImageIf.h"
#include "engine/graphics/Format.h"
#include "containers/vector.h"

namespace engine
{
	namespace image
	{
		class ExternalCodecs : public ImageIf
		{
		public:
			ExternalCodecs(const engine::string& filename);
			ExternalCodecs(const engine::string& filename,
				Format format,
				unsigned int width,
				unsigned int height,
				unsigned int slices,
				unsigned int mips);
			~ExternalCodecs();
			ExternalCodecs(const ExternalCodecs&) = default;
			ExternalCodecs(ExternalCodecs&&) = default;
			ExternalCodecs& operator=(const ExternalCodecs&) = default;
			ExternalCodecs& operator=(ExternalCodecs&&) = default;

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
			
			size_t m_width;
			size_t m_height;
			Format m_format;
			size_t m_mips;
			size_t m_slices;

			unsigned int m_channels;
			bool m_integerFormat;

			void allocateMemoryForImage();
			void convertData(const char* data, unsigned int stride);
			engine::vector<uint8_t> m_data;
		};
	}
}
