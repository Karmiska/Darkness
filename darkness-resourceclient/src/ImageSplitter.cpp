#include "ImageSplitter.h"
#include "tools/Debug.h"
#include "engine/rendering/Material.h"
#include "tools/ToolsCommon.h"
#include "platform/Uuid.h"
#include "engine/graphics/Common.h"

// we need the formats
#include "Compressonator.h"

#define  FREEIMAGE_LIB
#include "FreeImage.h"
#include "Utilities.h"

using namespace engine;

#define MaximumImageSize 8192

void freeImageOutput(FREE_IMAGE_FORMAT /*fif*/, const char *msg)
{
    LOG("%s", msg);
}

namespace resource_client
{
    ImageSplitter::ImageSplitter()
    {
        FreeImage_Initialise();
        FreeImage_SetOutputMessage(freeImageOutput);
    }

    ImageSplitter::~ImageSplitter()
    {
        FreeImage_DeInitialise();
    }

    engine::vector<TextureType> possibleTextureTypes(FREE_IMAGE_TYPE type, unsigned int bpp)
    {
        switch (type)
        {
        case FIT_UNKNOWN: ASSERT(false, "Unknown image type!");
        case FIT_BITMAP:
        {
            if (bpp == 32 || bpp == 24)
                return { TextureType::Albedo };
            else if (bpp == 8)
                return { TextureType::Roughness, TextureType::Metalness, TextureType::Ambient, TextureType::Height };
        }
        case FIT_UINT16: return { TextureType::Shininess, TextureType::Ambient, TextureType::Displacement, TextureType::Emissive, TextureType::Height, TextureType::Lightmap, TextureType::Opacity, TextureType::Reflection, TextureType::Roughness };
        case FIT_INT16: return { TextureType::Shininess, TextureType::Ambient, TextureType::Displacement, TextureType::Emissive, TextureType::Height, TextureType::Lightmap, TextureType::Opacity, TextureType::Reflection, TextureType::Roughness };
        case FIT_UINT32: return { TextureType::Shininess, TextureType::Ambient, TextureType::Displacement, TextureType::Emissive, TextureType::Height, TextureType::Lightmap, TextureType::Opacity, TextureType::Reflection, TextureType::Roughness };
        case FIT_INT32: return { TextureType::Shininess, TextureType::Ambient, TextureType::Displacement, TextureType::Emissive, TextureType::Height, TextureType::Lightmap, TextureType::Opacity, TextureType::Reflection, TextureType::Roughness };
        case FIT_FLOAT: return { TextureType::Shininess, TextureType::Ambient, TextureType::Displacement, TextureType::Emissive, TextureType::Height, TextureType::Lightmap, TextureType::Opacity, TextureType::Reflection, TextureType::Roughness };
        case FIT_DOUBLE: return { TextureType::Shininess, TextureType::Ambient, TextureType::Displacement, TextureType::Emissive, TextureType::Height, TextureType::Lightmap, TextureType::Opacity, TextureType::Reflection, TextureType::Roughness };
        case FIT_COMPLEX: return { TextureType::Shininess, TextureType::Ambient, TextureType::Displacement, TextureType::Emissive, TextureType::Height, TextureType::Lightmap, TextureType::Opacity, TextureType::Reflection, TextureType::Roughness };
        case FIT_RGB16: return { TextureType::Albedo, TextureType::Normal, TextureType::Hdr };
        case FIT_RGBA16: return { TextureType::Albedo, TextureType::Normal, TextureType::Hdr };
        case FIT_RGBF: return { TextureType::Hdr, TextureType::Albedo, TextureType::Normal };
        case FIT_RGBAF: return { TextureType::Hdr, TextureType::Albedo, TextureType::Normal };

        }
        return { TextureType::Albedo };
    }

    CMP_FORMAT compressonatorFormat(FREE_IMAGE_TYPE type, unsigned int bytesPerPixel)
    {
        switch (type)
        {
        case FIT_UNKNOWN: ASSERT(false, "Unknown image type!");
        case FIT_BITMAP:
        {
            if (bytesPerPixel == 24)
                return CMP_FORMAT_BGR_888;
            else if (bytesPerPixel == 32)
                return CMP_FORMAT_BGRA_8888;
            else if (bytesPerPixel == 8)
                return CMP_FORMAT_R_8;
            ASSERT(false, "Unsupported bytesPerPixel: %u", bytesPerPixel);
        }
        case FIT_UINT16: return CMP_FORMAT_R_16;
        case FIT_INT16: return CMP_FORMAT_R_16;
        case FIT_UINT32: return CMP_FORMAT_Unknown;
        case FIT_INT32: return CMP_FORMAT_Unknown;
        case FIT_FLOAT: return CMP_FORMAT_R_32F;
        case FIT_DOUBLE: return CMP_FORMAT_Unknown;
        case FIT_COMPLEX: return CMP_FORMAT_Unknown;
        case FIT_RGB16: return CMP_FORMAT_Unknown;
        case FIT_RGBA16: return CMP_FORMAT_RGBA_16;
        case FIT_RGBF: return CMP_FORMAT_RGB_32F;
        case FIT_RGBAF: return CMP_FORMAT_RGBA_32F;

        }
        return CMP_FORMAT_BGRA_16;
    }

    CMP_FORMAT compressonatorBlockFormat(engine::Format format)
    {
        switch (format)
        {
        case Format::BC1_UNORM: return CMP_FORMAT_BC1;
        case Format::BC3_UNORM: return CMP_FORMAT_BC3;
        case Format::BC4_UNORM: return CMP_FORMAT_BC4;
        case Format::BC5_UNORM: return CMP_FORMAT_BC5;
        case Format::BC6H_UF16: return CMP_FORMAT_BC6H;
        case Format::BC7_UNORM: return CMP_FORMAT_BC7;
        case Format::BC1_UNORM_SRGB: return CMP_FORMAT_BC1;
        case Format::BC3_UNORM_SRGB: return CMP_FORMAT_BC3;
        case Format::BC7_UNORM_SRGB: return CMP_FORMAT_BC7;
        default: return CMP_FORMAT_BC1;
        }
    }


    engine::vector<SplitTask> ImageSplitter::splitImageTask(Task& container)
    {
        engine::vector<SplitTask> result;

        FIMEMORY* imageMemory = FreeImage_OpenMemory(reinterpret_cast<BYTE*>(container.data.data()), static_cast<DWORD>(container.data.size()));

        FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileTypeFromMemory(imageMemory);
        FIBITMAP* bitmap = FreeImage_LoadFromMemory(imageFormat, imageMemory, 0);

        // extract everything
        unsigned int width = FreeImage_GetWidth(bitmap);
        unsigned int height = FreeImage_GetHeight(bitmap);

        auto imageType = FreeImage_GetImageType(bitmap);
        unsigned int bpp = FreeImage_GetBPP(bitmap);
        auto possibleTargetFormats = possibleEncodingFormatRGB(possibleTextureTypes(imageType, bpp));

        /*unsigned int redMask = FreeImage_GetRedMask(bitmap);
        unsigned int greenMask = FreeImage_GetGreenMask(bitmap);
        unsigned int blueMask = FreeImage_GetBlueMask(bitmap);*/
        //bool transparent = FreeImage_IsTransparent(bitmap);

        if (width > MaximumImageSize)
            width = MaximumImageSize;
        if (width % 4 != 0 || !isPowerOfTwo(width))
        {
            int potSize = 4;
            while (static_cast<unsigned int>(potSize) < width) potSize *= 2;
            width = potSize;
        }

        if (height > MaximumImageSize)
            height = MaximumImageSize;
        if (height % 4 != 0 || !isPowerOfTwo(height))
        {
            int potSize = 4;
            while (static_cast<unsigned int>(potSize) < height) potSize *= 2;
            height = potSize;
        }

        //auto encodeType = static_cast<Format>(container.image.format);
        //auto encodeType = static_cast<Format>(container.image.format) == Format::UNKNOWN ? possibleTargetFormats[0] : static_cast<Format>(container.image.format);
        auto encodeType = possibleTargetFormats[0];

        auto imageBytes = 0;
        auto w = width;
        auto h = height;
        int mipCount = 1;
        while (w > 1 && h > 1)
        {
            imageBytes += static_cast<int>(engine::formatBytes(encodeType, w, h));
            w /= 2;
            h /= 2;
            if (!container.image.generateMips)
                break;
            if (w > 1 && h > 1)
                ++mipCount;
        }

		constexpr int PartSize = 128;

        int workWidth = width;
        int workHeight = height;
        for (int i = 0; i < mipCount; ++i)
        {
            auto mipScaled = FreeImage_Rescale(bitmap, workWidth, workHeight, FREE_IMAGE_FILTER::FILTER_LANCZOS3);

            auto flipRes = FreeImage_FlipVertical(mipScaled);
            //ASSERT(flipRes, "Failed to flip image");

            auto imageDataPtr = FreeImage_GetBits(mipScaled);
            auto stride = FreeImage_GetPitch(mipScaled);

			/*auto horizBlockCount = roundUpToMultiple(workWidth, PartSize) / PartSize;
			auto vertBlockCount = roundUpToMultiple(workHeight, PartSize) / PartSize;

			for (int x = 0; x < horizBlockCount; ++x)
			{
				for (int y = 0; y < vertBlockCount; ++y)
				{

				}
			}*/
			
            SplitTask splitTask;
            splitTask.type = TaskType::Image;
            splitTask.progress = 0.0f;
            splitTask.bytes = workHeight * stride;
            splitTask.taskId = container.taskId;
            splitTask.subTaskId = platform::uuid();
            splitTask.image.width = workWidth;
            splitTask.image.height = workHeight;
            splitTask.image.stride = stride;
			splitTask.image.mipCount = mipCount;
			splitTask.image.mip = i;
			splitTask.image.partId = 0;
			splitTask.image.partWidth = 0;
			splitTask.image.partHeight = 0;
            splitTask.image.sourceFormat = static_cast<int>(compressonatorFormat(imageType, bpp));
            splitTask.image.targetFormat = static_cast<int>(compressonatorBlockFormat(encodeType));
            splitTask.image.originalFormat = static_cast<int>(encodeType);
            splitTask.data.resize(workHeight * stride);
            memcpy(&splitTask.data[0], imageDataPtr, splitTask.data.size());
            result.emplace_back(std::move(splitTask));

            if (!container.image.generateMips)
                break;

            workWidth /= 2;
            workHeight /= 2;
        }
        FreeImage_Unload(bitmap);

        FreeImage_CloseMemory(imageMemory);

        return result;
    }

    TaskResult ImageSplitter::joinImageTask(engine::vector<SplitTaskResult>& results)
    {
        // sort results
        auto sortRule = [](const SplitTaskResult& a, const SplitTaskResult& b)->bool { return a.image.width > b.image.width; };
        std::sort(results.begin(), results.end(), sortRule);

        int width = results.front().image.width;
        int height = results.front().image.height;
        int format = results.front().image.format;
		//int mipCount = results.front().image.mipCount;
        int mips = mipCount(width, height);

        //
        auto encodeType = static_cast<Format>(format);
        auto settings_generateMips = results.size() > 1;

        auto imageBytes = 0;
        auto w = width;
        auto h = height;
        while (w > 1 && h > 1)
        {
            imageBytes += static_cast<int>(engine::formatBytes(encodeType, w, h));
            w /= 2;
            h /= 2;
            if (!settings_generateMips)
                break;
        }

        engine::vector<char> dstBuffer(imageBytes);
        uint32_t currentPoint = 0;
        for (auto&& res : results)
        {
            memcpy(&dstBuffer[currentPoint], res.image.data.data(), res.image.data.size());
            currentPoint += static_cast<uint32_t>(res.image.data.size());
        }
        TaskResult res;
        res.type = TaskType::Image;
        res.image.data = std::move(dstBuffer);
        res.image.width = width;
        res.image.height = height;
        res.image.mips = mips;
        res.image.format = format;
        return res;
    }
}
