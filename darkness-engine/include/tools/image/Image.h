#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include "engine/primitives/Color.h"
#include "ImageIf.h"

// Factory class to create any supported image

namespace engine
{
    namespace image
    {
        enum class ImageType
        {
            DDS,
            BMP,
			EXTERNAL
        };

        class Image
        {
        public:
            static engine::shared_ptr<ImageIf> createImage(
                const engine::string& filename,
                ImageType imageType,
                Format type = Format::BC7_UNORM,

                int width = -1,
                int height = -1,
                int slices = -1,
                int mips = -1);
        };

    }
}
