#include "tools/image/Image.h"
#include "tools/image/Dds.h"
#include "tools/image/Bmp.h"
#include "tools/image/ExternalCodecs.h"
#include "tools/PathTools.h"
#include "platform/File.h"
#include "engine/filesystem/VirtualFilesystem.h"
#include <algorithm>

using namespace engine;

namespace engine
{
    namespace image
    {
		engine::shared_ptr<ImageIf> Image::createImage(
            const engine::string& filename,
            ImageType imageType,
            Format format,
            int width,
            int height,
            int slices,
            int mips)
        {
			auto sysPath = resolvePath(filename);
            if (width != -1 && height != -1)
            {
                if (imageType == ImageType::DDS)
                {
                    return engine::make_shared<Dds>(
						sysPath,
                        format,
                        static_cast<unsigned int>(width), static_cast<unsigned int>(height),
                        slices != -1 ? slices : 1,
                        mips != -1 ? mips : 1
                        );
                }
                else if (imageType == ImageType::BMP)
                {
                    return engine::make_shared<Bmp>(
						sysPath,
                        format,
                        static_cast<unsigned int>(width), static_cast<unsigned int>(height),
                        slices != -1 ? slices : 1,
                        mips != -1 ? mips : 1
                        );
                }
            }
            else
            {
                if (!fileExists(sysPath))
                    return nullptr;

                auto ext = pathExtractExtension(sysPath);
                std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                if (ext == "dds" || imageType == ImageType::DDS)
                {
                    return engine::make_shared<Dds>(sysPath);
                }
                else if (imageType == ImageType::BMP)
                {
                    return engine::make_shared<Bmp>(sysPath);
                }
				else if (imageType == ImageType::EXTERNAL)
				{
					return engine::make_shared<ExternalCodecs>(sysPath);
				}
            }
            return nullptr;
        }

    }
}
